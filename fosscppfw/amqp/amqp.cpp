#include "amqp.h"

#include <algorithm>
#include <amqpcpp.h>
#include <array>
#include <memory>
#include <stdexcept>
#include <zstd.h>

namespace AMQP {

PreparedReply preparePlainReply(ReplyResult result) {
	if (auto* text = std::get_if<std::string>(&result)) return {std::move(*text), {}, {}};
	return std::get<ReplyProducer>(result)(Table{}, ReplyCompression::None);
}

PreparedReply prepareReplyResult(
	ReplyResult result, Table const& requestHeaders, ReplyCompression compression,
	ReplyPreparer const& preparer, std::string const& correlationId, size_t maxFrameBytes,
	ReplyPreparationErrorFactory const& errorFactory
) {
	std::optional<PreparedReply> prepared;
	if (auto* text = std::get_if<std::string>(&result)) {
		prepared = prepareReply(*text, requestHeaders, compression, preparer, correlationId, maxFrameBytes, errorFactory);
	} else {
		auto const& producer = std::get<ReplyProducer>(result);
		prepared = prepareReply({}, requestHeaders, compression,
			[&producer](std::string const&, Table const& headers, ReplyCompression encoding) {
				return producer(headers, encoding);
			}, correlationId, maxFrameBytes, errorFactory);
	}
	return prepared ? std::move(*prepared) : preparePlainReply(std::move(result));
}

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
		auto const pledged = ZSTD_CCtx_setPledgedSrcSize(context.get(), payload.size());
		if (ZSTD_isError(pledged)) return failed(ZSTD_getErrorName(pledged));
		// Grow with compressed output, not the potentially huge uncompressed reply.
		std::string compressed;
		std::array<char, 64 * 1024> buffer;
		ZSTD_inBuffer input {payload.data(), payload.size(), 0};
		size_t remaining;
		do {
			ZSTD_outBuffer output {buffer.data(), buffer.size(), 0};
			remaining = ZSTD_compressStream2(context.get(), &output, &input, ZSTD_e_end);
			if (ZSTD_isError(remaining)) return failed(ZSTD_getErrorName(remaining));
			if (!required && compressed.size() + output.pos >= payload.size()) return std::nullopt;
			compressed.append(buffer.data(), output.pos);
		} while (remaining != 0);
		compressed.shrink_to_fit();
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
		if (!prepared || prepared->headers.empty()) return prepared;
		Table headers;
		for (auto const& entry : prepared->headers) headers[entry.first] = entry.second;
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
