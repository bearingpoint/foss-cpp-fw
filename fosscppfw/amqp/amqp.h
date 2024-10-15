#pragma once

#include <string>
#include <vector>
#include <functional>

namespace AMQP {

using MQResultCallback = std::function<void(std::string)>;
using MQHandler = std::function<void(std::string payload, MQResultCallback resultCallback)>;

namespace detail {
	struct BaseConfig {
		std::string name = "";
		bool durable = true;
		bool autodelete = false;
		/** optionally specify a message TTL (in milliseconds). Ignored if <= 0 */
		int messageTtl = 0;

		BaseConfig() = default;

		BaseConfig(std::string name, bool durable, bool autodelete, int messageTtl):
			name(name), durable(durable), autodelete(autodelete), messageTtl(messageTtl)
		{}
	};
} // namespace detail

struct QueueConfig: public detail::BaseConfig {
	struct Binding {
		std::string exchange;
		std::string routingKey;
	} exchangeBinding;
	MQHandler handler = nullptr;

	QueueConfig() = default;

	QueueConfig(std::string name, bool durable, bool autodelete, int messageTtl, std::string bindToExchange = "", std::string routingKey = "", MQHandler handler = nullptr):
		BaseConfig(name, durable, autodelete, messageTtl),
		exchangeBinding { bindToExchange, routingKey },
		handler(handler)
	{}
	// FIXME builder pattern
};
struct ExchangeConfig: detail::BaseConfig {
	std::string type = "";
	// int xRandomQueues = 1;
	// AMQPManager::MQHandler xRandomQueuesHandler;
	// std::vector<QueueConfig> queues = {};

	ExchangeConfig() = default;

	ExchangeConfig(std::string name, bool durable, bool autodelete, int messageTtl,
		std::string type, int xRandomQueues, MQHandler xRandomQueuesHandler = nullptr, std::vector<QueueConfig> queues = {}
	)
		: BaseConfig(name, durable, autodelete, messageTtl), type(type)
		// , xRandomQueues(xRandomQueues)
		// , xRandomQueuesHandler(xRandomQueuesHandler)
		// , queues(queues)
	{}
};

struct ConnectionConfig {
	int channelPrefetch;
	std::string host;
	int port;
	std::string username;
	std::string password;
	int heartbeatInterval;

	ConnectionConfig(
		int channelPrefetch = 1,
		std::string host = "localhost",
		int port = 5672,
		std::string username = "guest",
		std::string password = "guest",
		int heartbeatInterval = 60
	):
		channelPrefetch(channelPrefetch),
		host(host),
		port(port),
		username(username),
		password(password),
		heartbeatInterval(heartbeatInterval)
	{};
};

/**
 * Use this utility to generate [count] queue configs for an x-random exchange, all with the same handler
 * The names of the queues will be "exchangeName-queue-1", "exchangeName-queue-2", etc.
 */
std::vector<QueueConfig> generateXRandomQueues(std::string const& exchangeName, int count, MQHandler handler);

} // namespace AMQP
