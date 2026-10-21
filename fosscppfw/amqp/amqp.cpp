#include "amqp.h"

#include <algorithm>
#include <amqpcpp.h>
#include <memory>
#include <limits>
#include <random>
#include <stdexcept>
#include <zstd.h>

namespace AMQP {

ReplyCompression parseReplyCompression(std::string_view value) {
	if (value == "none") return ReplyCompression::None;
	if (value == "zstd") return ReplyCompression::Zstd;
	throw std::invalid_argument("Reply compression must be 'none' or 'zstd'");
}

ReplyCompression negotiateReplyCompression(ReplyCompression configured, Table const& headers) {
	auto const& acceptedEncoding = headers.get("x-accept-reply-encoding");
	return acceptedEncoding.isString() && static_cast<std::string const&>(acceptedEncoding) == "zstd"
		? configured : ReplyCompression::None;
}

std::optional<std::string> compressReply(
	std::string_view payload, ReplyCompression compression, ReplyCompressionPolicy policy
) {
	constexpr size_t maxReplyBytes = 64 * 1024 * 1024;
	bool const required = policy == ReplyCompressionPolicy::Required;
	if (compression != ReplyCompression::Zstd) {
		if (required) throw std::invalid_argument("Required reply compression is disabled");
		return std::nullopt;
	}
	if (!required && (payload.size() < 2 || payload.size() > maxReplyBytes)) return std::nullopt;
	auto failed = [required](char const* detail) -> std::optional<std::string> {
		if (required) throw std::runtime_error(std::string("Reply compression failed: ") + detail);
		return std::nullopt;
	};
	try {
		std::unique_ptr<ZSTD_CCtx, decltype(&ZSTD_freeCCtx)> context(ZSTD_createCCtx(), ZSTD_freeCCtx);
		if (!context
			|| ZSTD_isError(ZSTD_CCtx_setParameter(context.get(), ZSTD_c_compressionLevel, 1))
			|| ZSTD_isError(ZSTD_CCtx_setParameter(context.get(), ZSTD_c_contentSizeFlag, 1))
			|| ZSTD_isError(ZSTD_CCtx_setParameter(context.get(), ZSTD_c_checksumFlag, 1))) {
			return failed("unable to initialize Zstandard");
		}
		size_t const capacity = required ? ZSTD_compressBound(payload.size()) : payload.size() - 1;
		if (ZSTD_isError(capacity)) return failed(ZSTD_getErrorName(capacity));
		std::string compressed(capacity, '\0');
		size_t const bytes = ZSTD_compress2(
			context.get(), compressed.data(), compressed.size(), payload.data(), payload.size()
		);
		if (ZSTD_isError(bytes)) return failed(ZSTD_getErrorName(bytes));
		compressed.resize(bytes);
		return compressed;
	} catch (std::bad_alloc const&) {
		if (required) throw;
		return std::nullopt;
	} catch (std::length_error const&) {
		if (required) throw;
		return std::nullopt;
	}
}

size_t getReplyChunkSize(std::string_view remainingPayload, size_t maxChunkBytes, bool preserveUtf8Boundaries) {
	size_t const chunkSize = std::min(remainingPayload.size(), maxChunkBytes);
	if (!preserveUtf8Boundaries || chunkSize == remainingPayload.size() || chunkSize == 0) {
		return chunkSize;
	}
	auto isContinuation = [&remainingPayload](size_t offset) {
		return (static_cast<unsigned char>(remainingPayload[offset]) & 0xc0) == 0x80;
	};
	size_t boundary = chunkSize;
	// A valid UTF-8 character has at most three continuation bytes.
	while (boundary > 0 && chunkSize - boundary < 3 && isContinuation(boundary)) {
		--boundary;
	}
	// Invalid byte sequences still make progress instead of producing an empty chunk.
	return boundary > 0 && !isContinuation(boundary) ? boundary : chunkSize;
}

void setReplyChunkHeaders(Table& headers, size_t index, size_t count, size_t totalBytes) {
	headers["x-reply-chunk-index"] = std::to_string(index);
	headers["x-reply-chunk-count"] = std::to_string(count);
	headers["x-reply-total-bytes"] = std::to_string(totalBytes);
}

static std::string replyAttemptId() {
	std::random_device random;
	char const* hex = "0123456789abcdef";
	std::string id;
	for (size_t i = 0; i < 16; ++i) {
		auto const byte = random() & 0xff;
		id += hex[byte >> 4];
		id += hex[byte & 0xf];
	}
	return id;
}

std::optional<PreparedReply> prepareReply(
	std::string const& payload, Table const& requestHeaders, ReplyCompression compression,
	ReplyPreparer const& preparer, std::string const& correlationId, size_t maxFrameBytes,
	ReplyPreparationErrorFactory const& errorFactory
) {
	try {
		std::optional<PreparedReply> prepared;
		if (preparer) {
			prepared = preparer(payload, requestHeaders, compression);
		} else if (auto compressed = compressReply(payload, negotiateReplyCompression(compression, requestHeaders))) {
			prepared = PreparedReply {std::move(*compressed), {}, "zstd"};
		}
		if (!prepared) return prepared;
		if (prepared->sequenceChunks) {
			if (prepared->contentEncoding != "zstd") throw std::invalid_argument("Sequenced replies require binary Zstandard chunks");
			prepared->headers.emplace_back("x-reply-chunk-protocol", "1");
			prepared->headers.emplace_back("x-reply-attempt-id", replyAttemptId());
		}
		if (prepared->headers.empty()) return prepared;
		Table headers;
		for (auto const& entry : prepared->headers) headers[entry.first] = entry.second;
		// Reserve the largest possible decimal counters before publishing any part.
		if (prepared->sequenceChunks) {
			auto const maximum = std::numeric_limits<size_t>::max();
			setReplyChunkHeaders(headers, maximum, maximum, maximum);
		}
		MetaData metadata;
		metadata.setHeaders(headers);
		metadata.setContentEncoding(prepared->contentEncoding);
		metadata.setCorrelationID(correlationId);
		metadata.setTypeName("multipart/incomplete");
		// AMQP content-header framing: frame prefix/end (8), class/weight (4), body size (8).
		if (maxFrameBytes != 0 && 20u + metadata.size() > maxFrameBytes) {
			if (!errorFactory) return std::nullopt;
			throw std::length_error("Reply metadata exceeds the negotiated AMQP frame size");
		}
		return prepared;
	} catch (std::exception const& error) {
		if (!errorFactory) throw;
		return PreparedReply {errorFactory(error.what()), {}, {}};
	}
}

std::vector<QueueConfig> generateXRandomQueues(std::string const& exchangeName, int count, MQHandler handler) {
	std::vector<QueueConfig> queues;
	if (count <= 0) {
		return {};
	}
	for (int i = 0; i < count; i++) {
		queues.push_back(
			QueueConfig(exchangeName + "-queue-" + std::to_string(i + 1))
				.setExchangeBinding(exchangeName, "")
				.setHandler(handler)
		);
	}
	return queues;
}

} // namespace AMQP
