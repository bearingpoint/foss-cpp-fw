/*
	AMQP manager manages AMQP connection and queue transactions

	Copyright: Bearingpoint
	Created: 09-03-2020
	Author: Bogdan Ionita <bogdan.ionita@bearingpoint.com>
*/

#include "amqp-manager.h"

#include "../utils/log.h"
#include "../net/connection.h"
#include "../utils/ioModif.h"
#include "../utils/strbld.h"
#include "../perf/marker.h"

#include <sstream>
#include <chrono>
#include <thread>

//#define ENABLE_DEBUG_AMQP_LOGS // uncomment to enable debug logs
#define AMQP_LOG_COLOR ioModif::FG_GREEN

#if defined(DEBUG) && defined(ENABLE_DEBUG_AMQP_LOGS)
#define DEBUGAMQPLOG(X) DEBUGLOGLN(AMQP_LOG_COLOR << X)
#else
#define DEBUGAMQPLOG(X)
#endif

#define AMQPLOGLN(X) LOGLN(AMQP_LOG_COLOR << "[AMQP] " << X)

static const size_t MAX_SEND_FRAME_SIZE = 4096; // bytes

unsigned AMQPManager::globalPrefetch_ = 0;
unsigned AMQPManager::channelPrefetchCount_ = 1;

void printConnectionStatus(AMQP::Connection *connection) {
	DEBUGAMQPLOG("Connection status: +++++++++++++++++++++++++++++\n"
		<< "vhost: " << connection->vhost() << "\n"
		<< "login: " << connection->login() << "\n"
		<< "channels: " << connection->channels() << "\n"
		<< "initialized: " << connection->initialized() << "\n"
		<< "ready: " << connection->ready() << "\n"
		<< "usable: " << connection->usable() << "\n"
		<< "waiting: " << connection->waiting() << "\n"
	);
}

AMQPManager::AMQPManager(
	std::vector<AMQPManager::QueueConfig> mqQueues,
	AMQPManager::ConnectionConfig connectionConfig
)
	: connectionConfig_(connectionConfig)
	, queues_(std::move(mqQueues))
{}

AMQPManager::AMQPManager(
	std::vector<ExchangeConfig> mqExchanges,
	AMQPManager::ConnectionConfig connectionConfig
)
	: connectionConfig_(connectionConfig)
	, exchanges_(std::move(mqExchanges))
{}

AMQPManager::AMQPManager(
	std::vector<QueueConfig> mqQueues,
	std::vector<ExchangeConfig> mqExchanges,
	AMQPManager::ConnectionConfig connectionConfig
)
	: connectionConfig_(connectionConfig)
	, queues_(std::move(mqQueues))
	, exchanges_(std::move(mqExchanges))
{}

AMQPManager::AMQPManager() {}

AMQPManager::~AMQPManager() {
	if (amqpChannel_) {
		delete amqpChannel_;
		amqpChannel_ = nullptr;
	}
	if (amqpConnection_)
		delete amqpConnection_;
	if (buffer_) {
		free(buffer_);
		buffer_ = nullptr;
	}
}

void AMQPManager::run(std::function<void()> idleCallback) {
	// set up the socket connection to the RabbitMQ server
	reconnect();

	amqpMaxFrameSize_ = amqpConnection_->maxFrame();
	bufferSize_ = 2 * amqpMaxFrameSize_;
	buffer_ = (char*)malloc(bufferSize_);

	// start the loop
	while (true) {
		try {
			const size_t expectedDataSize = amqpConnection_->expected();
			bool socketIdle = false;
			if (bufferWriteOffset_ - bufferReadOffset_ < expectedDataSize) {
				if (expectedDataSize + bufferWriteOffset_ > bufferSize_) {
					// we must increase the buffer to accommodate more data
					bufferSize_ = std::max(2 * bufferSize_, expectedDataSize + bufferWriteOffset_);
					void* newBuffer = realloc(buffer_, bufferSize_);
					if (!newBuffer) {
						// failed to realloc
						throw std::runtime_error("Failed to realloc a larger buffer for AMQP");
					}
					buffer_ = (char*)newBuffer;
				}
				size_t bytesToRead = std::min(net::bytesAvailable(sockConn_), expectedDataSize);
				if (bytesToRead) {
					try {
						net::result res = net::read(sockConn_, buffer_ + bufferWriteOffset_, bufferSize_ - bufferWriteOffset_, bytesToRead);
						if (res != net::result::ok) {
							throw std::runtime_error(strbld() << "Error while reading from socket: " << net::errorString(res));
						}
						timeLastDataReceived_ = std::chrono::system_clock::now();
					} catch (std::exception &e) {
						ERRORLOG("Exception reading " << bytesToRead << " bytes from socket: " << e.what());
						throw e;
					}
					bufferWriteOffset_ += bytesToRead;
				} else {
					socketIdle = true;
				}
			}
			const size_t sizeToParse = std::min(expectedDataSize, bufferWriteOffset_ - bufferReadOffset_);
			if (sizeToParse > 0) {
				DEBUGAMQPLOG(EM_ON << "Parsing " << sizeToParse << " bytes of AMQP data." << EM_OFF);
				size_t parsedSize;
				try {
					parsedSize = amqpConnection_->parse(buffer_ + bufferReadOffset_, sizeToParse);
				} catch (std::exception const& err) {
					ERRORLOG("Failed to parse AMQP data (sizeToParse: " << sizeToParse << "): " << err.what());
					throw err;
				}
				bufferReadOffset_ += parsedSize;
				if (bufferSize_ - bufferWriteOffset_ < bufferReadOffset_) {
					// there's more unused buffer to the left of the used portion than to the right.
					// we move everything to the beginning of the buffer
					// (there may still be some additional unread data between bufferReadOffset_ and bufferWriteOffset_ that we must not lose)
					auto const oldDataStart = bufferReadOffset_;
					auto const oldDataSize = (bufferWriteOffset_ > bufferReadOffset_) ? bufferWriteOffset_ - bufferReadOffset_ : 0;
					bufferWriteOffset_ = 0;
					bufferReadOffset_ = 0;
					if (oldDataSize) {
						DEBUGAMQPLOG("Recycling " << oldDataSize << " bytes of data while rotating the buffer.");
						memcpy(buffer_, buffer_ + oldDataStart, oldDataSize);
						bufferWriteOffset_ = oldDataSize;
					}
				}
			} else if (socketIdle) {
				// seems we're idle
				verifyTimeouts();
				if (idleCallback) {
					idleCallback();
				}
			}
		} catch (std::exception &e) {
			ERRORLOG("Exception while reading or parsing AMQP data: " << e.what());
			AMQPLOGLN("Error encountered, reconnecting...");
			reconnect();
		}
	}
}

void AMQPManager::reconnect() {
	LOGPREFIX("AMQPManager");
	AMQPLOGLN("Connecting...")
	if (socketConnected_) {
		net::closeConnection(sockConn_);
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	initSocketConnection();
	if (amqpChannel_) {
		delete amqpChannel_;
		amqpChannel_ = nullptr;
	}
	if (amqpConnection_) {
		delete amqpConnection_;
		amqpConnection_ = nullptr;
	}
	// set up AMQP protocol
	amqpConnection_ = new AMQP::Connection(this, AMQP::Login(connectionConfig_.username, connectionConfig_.password));
	amqpChannel_ = new AMQP::Channel(amqpConnection_);
	// prefetch per connection -> 1 to avoid messages getting "reserved" by busy threads while other threads rub the peppermint.
	amqpChannel_->setQos(channelPrefetchCount_, false);
	// global prefetch shared between all consumers
	amqpChannel_->setQos(globalPrefetch_, true);
	amqpChannel_->onError([this] (const char* err) {
		throw std::runtime_error(std::string("AMQP Channel error: ") + err);
	});
	amqpChannel_->onReady([]() {
		DEBUGAMQPLOG("Channel is READY.");
	});
	setupQueues(queues_);
	setupExchanges(exchanges_);
	AMQPLOGLN("Queues and exchanges set up.");

	timeLastHeartbeatSent_ = std::chrono::system_clock::now();
	timeLastDataReceived_ = std::chrono::system_clock::now();
	bufferReadOffset_ = bufferWriteOffset_ = 0;
}

void AMQPManager::initSocketConnection() {
	socketConnected_ = false;
	do {
		AMQPLOGLN("Connecting to RabbitMQ...");
		net::result res = net::connect(connectionConfig_.host, connectionConfig_.port, sockConn_);
		if (res != net::result::ok) {
			ERRORLOG("Unable to connect to RabbitMQ server at " << connectionConfig_.host << ":" << connectionConfig_.port
				<< "\n" << net::errorString(res));
			ERRORLOG("Sleeping for 10 seconds, then we'll retry...");
			std::this_thread::sleep_for(std::chrono::seconds(10));
		} else {
			AMQPLOGLN("Connected successfully to RabbitMq.");
			socketConnected_ = true;
		}
	} while (!socketConnected_);
}

void AMQPManager::setupQueues(std::vector<QueueConfig> const& queues) {
	// declare queues and install queue handlers provided by caller
	for (auto &qConfig : queues) {
		int queueFlags = 0;
		AMQP::Table queueOptions;
		if (qConfig.durable) {
			queueFlags |= AMQP::durable;
		}
		if (qConfig.autodelete) {
			queueFlags |= AMQP::autodelete;
			queueOptions["x-expires"] = 5 * 60 * 1000; // 5 minutes max
		}
		if (qConfig.messageTtl > 0) {
			queueOptions["x-message-ttl"] = qConfig.messageTtl;
		}
		amqpChannel_->declareQueue(qConfig.name, queueFlags, queueOptions)
			.onSuccess([this, &qConfig] (std::string const& qName, uint32_t msgCount, uint32_t csmCount)
		{
			DEBUGAMQPLOG("Queue declared: " << qName);
			amqpChannel_->consume(qName).onReceived([this, qName, &qConfig](AMQP::Message const &msg, uint64_t deliveryTag, bool redelivered) {
				// decode the message
				auto payload = std::string(msg.body(), msg.body() + msg.bodySize());
				DEBUGAMQPLOG("Received MQ message on queue '" << qName << "'; correlationId: " << msg.correlationID());

				// extract useful data from the message:
				std::string replyTo = msg.replyTo();
				std::string correlationId = msg.correlationID();
				// call the handler
				qConfig.handler(payload, [this, deliveryTag, replyTo, correlationId] (std::string result) {
					PERF_MARKER("AMQP-send-reply");
					// this is the result callback, which should ALWAYS be invoked from the main thread
					DEBUGAMQPLOG("Sending back reply on queue '" << replyTo << "'");
					AMQP::Envelope envelope(result.c_str(), result.size());
					envelope.setCorrelationID(correlationId);
					// send the reply
					amqpChannel_->publish("", replyTo, envelope, AMQP::mandatory);
					// acknowledge the initial message so it can be removed from the queue
					amqpChannel_->ack(deliveryTag);
				});
			});
		});
	}
}

void AMQPManager::addXRandomQueuesToExchange(AMQPManager::ExchangeConfig &outExchange) {
	outExchange.queues = {};
	if (!outExchange.xRandomQueues) {
		outExchange.xRandomQueues = 1;
	}
	for (int i = 0; i < outExchange.xRandomQueues; i++) {
		outExchange.queues.push_back(
			AMQPManager::QueueConfig {
				outExchange.name + "-queue-" + std::to_string(i + 1),
				true,
				false,
				0,
				"",
				outExchange.xRandomQueuesHandler
			}
		);
	}
}

void AMQPManager::setupExchanges(std::vector<ExchangeConfig> &exchanges) {
	// declare queues and install queue handlers provided by caller
	for (auto &exchange : exchanges) {
		if (!exchange.name.empty()) {
			if (exchange.type == "x-random") {
				addXRandomQueuesToExchange(exchange);
			}
			setupQueues(exchange.queues);
			for (auto& queue: exchange.queues) {
				AMQPLOGLN("Bind Queue " << queue.name << " to exchange " << exchange.name << " on routing-key  " << queue.routingKey);
				amqpChannel_->bindQueue(exchange.name, queue.name, queue.routingKey);
			}
		}
	}
}

void AMQPManager::verifyTimeouts() {
	int timeSinceLastDataReceived = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - timeLastDataReceived_).count();
	if (timeSinceLastDataReceived > 5 * connectionConfig_.heartbeatInterval) {
		// something seems wrong, we haven't received anything in a while, so let's reconnect
		AMQPLOGLN("No heartbeat received in " << 5 * connectionConfig_.heartbeatInterval << " seconds, forcing reconnect.");
		reconnect();
	}
	int timeSinceLastHeartbeat = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - timeLastHeartbeatSent_).count();
	if (timeSinceLastHeartbeat >= connectionConfig_.heartbeatInterval) {
		amqpConnection_->heartbeat();
		timeLastHeartbeatSent_ = std::chrono::system_clock::now();
	}
}

// AMQP::ConnectionHandler methods follow :::::::::::::::::::::::

void AMQPManager::onData(AMQP::Connection *connection, const char *data, size_t size) {
	DEBUGAMQPLOG(EM_ON << "AMQP send data (size " << size << ")" << EM_OFF);
	size_t sentSize = 0;
	while (sentSize < size) {
		size_t frameSize = std::min(MAX_SEND_FRAME_SIZE, std::min(amqpMaxFrameSize_, size - sentSize));
		bool success = false;
		try {
			auto res = net::write(sockConn_, data, frameSize);
			if (res != net::result::ok) {
				ERRORLOG("Fail sending data to RabbitMQ (error): " << net::errorString(res));
				ERRORLOG("Data size: " << size << "; frame size: " << frameSize);
			} else {
				success = true;
			}
		} catch (std::exception const& e) {
			ERRORLOG("Fail sending data to RabbitMQ (exception): " << e.what());
		}
		if (success) {
			// reset heartbeat time since any write to the socket counts as a "heartbeat"
			// timeLastHeartbeatSent_ = std::chrono::system_clock::now(); // It seems the above assumption was wrong - the server closes the connection if we don't heartbeat
		} else {
			AMQPLOGLN("Attempting to reconnect...");
			reconnect();
			return;
		}
		sentSize += frameSize;
		data += frameSize;
	}
}

void AMQPManager::onReady(AMQP::Connection *connection) {
	DEBUGAMQPLOG("connection::onready()");
	printConnectionStatus(connection);
}

void AMQPManager::onError(AMQP::Connection *connection, const char *message) {
	ERRORLOG("AMQP disconnected: " << message);
	printConnectionStatus(connection);
	AMQPLOGLN("Attempting to reconnect...");
	reconnect();
}

void AMQPManager::onClosed(AMQP::Connection *connection) {
	onError(connection, "Connection closed by peer.");
}

uint16_t AMQPManager::onNegotiate(AMQP::Connection *connection, uint16_t interval) {
	return connectionConfig_.heartbeatInterval;
}

void AMQPManager::onHeartbeat(AMQP::Connection *connection) {
	timeLastDataReceived_ = std::chrono::system_clock::now();
	DEBUGAMQPLOG("Heartbeat received");
}

void AMQPManager::setGlobalPrefetchForThreads(unsigned numberOfThreads, int prefetchCount) {
	globalPrefetch_ = numberOfThreads * prefetchCount * 1.5;
	channelPrefetchCount_ = prefetchCount > 0 ? prefetchCount : 1;
}
