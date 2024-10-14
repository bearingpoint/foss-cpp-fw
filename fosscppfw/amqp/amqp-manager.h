/*
	AMQP manager manages AMQP connection and queue transactions

	Copyright: Bearingpoint
	Created: 09-03-2020
	Author: Bogdan Ionita <bogdan.ionita@bearingpoint.com>
*/

#pragma once

#include <amqpcpp.h>

#include <functional>
#include <chrono>

/**
 * This class IS NOT THREAD SAFE !!!
 * Create it and call its methods ON THE SAME THREAD ALWAYS !!!
 * This is due to the nature of the underlying AMQP library which does not support multi-threading.
*/
class AMQPManager : private AMQP::ConnectionHandler {
public:
	using MQResultCallback = std::function<void(std::string)>;
	using MQHandler = std::function<void(std::string payload, MQResultCallback resultCallback)>;

	struct BaseConfig {
		std::string name;
		bool durable = true;
		bool autodelete = false;
		/** optionally specify a message TTL (in milliseconds). Ignored if <= 0 */
		int messageTtl = 0;

		BaseConfig(std::string name, bool durable, bool autodelete, int messageTtl):
			name(name), durable(durable), autodelete(autodelete), messageTtl(messageTtl)
		{}
	};

	struct QueueConfig: public BaseConfig {
		std::string routingKey;
		AMQPManager::MQHandler handler;

		QueueConfig(std::string name, bool durable, bool autodelete, int messageTtl, std::string routingKey = "", AMQPManager::MQHandler handler = nullptr):
			BaseConfig(name, durable, autodelete, messageTtl),
			routingKey(routingKey),
			handler(handler)
		{}
	};
	struct ExchangeConfig: BaseConfig {
		std::string type = "";
		unsigned xRandomQueues = 1;
		AMQPManager::MQHandler xRandomQueuesHandler;
		std::vector<QueueConfig> queues = {};

		ExchangeConfig(std::string name, bool durable, bool autodelete, int messageTtl,
			std::string type, unsigned xRandomQueues, AMQPManager::MQHandler xRandomQueuesHandler = nullptr, std::vector<QueueConfig> queues = {}
		):
			BaseConfig(name, durable, autodelete, messageTtl), type(type), xRandomQueues(xRandomQueues), xRandomQueuesHandler(xRandomQueuesHandler),
			queues(queues)
		{}
	};

	struct ConnectionConfig {
		std::string host = "localhost";
		int port = 5672;
		std::string username = "guest";
		std::string password = "guest";
		int heartbeatInterval = 60;

		ConnectionConfig() {};
		ConnectionConfig(std::string host):
			host(host)
		{};

		ConnectionConfig(std::string host, int port, std::string username, std::string password, int heartbeatInterval):
			host(host),
			port(port),
			username(username),
			password(password),
			heartbeatInterval(heartbeatInterval)
		{};
	};

	/**
	 * Sets the global channel prefetch count optimally given the number of threads that the application will use.
	 * This must be called before any instances of AMQPManager are created, and will impact all queues.
	 */
	static void setGlobalPrefetchForThreads(unsigned numberOfThreads, int prefetchCount);

	AMQPManager(std::vector<QueueConfig> mqQueues, ConnectionConfig connectionConfig = ConnectionConfig());
	AMQPManager(std::vector<ExchangeConfig> mqExchanges, ConnectionConfig connectionConfig = ConnectionConfig());
	AMQPManager(
		std::vector<QueueConfig> mqQueues,
		std::vector<ExchangeConfig> mqExchanges,
		ConnectionConfig connectionConfig = ConnectionConfig()
	);

	AMQPManager();
	~AMQPManager();

	void run(std::function<void()> idleCallback = nullptr);

	/**
	 * Process any pending data and performs network communication with the AMQP server.
	 * All messages are being sent and received during this function.
	 * Message handler callbacks are called from this function.
	 *
	 * !!! This function must be called in a loop at short intervals throughout the application's lifetime !!!
	 *
	 * @param block if true, this function will block until data can be read from the socket, otherwise it will
	 * return immediately with "false" as the return value. Pass true when you have nothing else to do and want
	 * to wait until there is data on the socket
	 *
	 * @returns true if any data was processed, false otherwise (if the queue is idle at the moment)
	*/
	// bool step(bool block);

private:
	void setupQueues(std::vector<QueueConfig> const& queues);
	void setupExchanges(std::vector<ExchangeConfig> &exchanges);
	void initSocketConnection();
	void reconnect();
	void verifyTimeouts();

	ConnectionConfig connectionConfig_;
	unsigned sockConn_ = 0;
	bool socketConnected_ = false;
	AMQP::Connection *amqpConnection_ = nullptr;
	AMQP::Channel* amqpChannel_ = nullptr;
	std::vector<QueueConfig> queues_;
	std::vector<ExchangeConfig> exchanges_;
	// bool reconnectRequired_  = false;
	std::chrono::system_clock::time_point timeLastHeartbeatSent_;
	std::chrono::system_clock::time_point timeLastDataReceived_;
	static unsigned globalPrefetch_;
	static unsigned channelPrefetchCount_;

	size_t bufferReadOffset_ = 0;
	size_t bufferWriteOffset_ = 0;
	size_t amqpMaxFrameSize_ = 256; // just a safe number to make sure we don't exceed the frame size while negociating. It will be changed after that
	size_t bufferSize_ = 0;
	char* buffer_ = nullptr;

	void addXRandomQueuesToExchange(AMQPManager::ExchangeConfig &outExchange);
	/**************** AMQP::ConnectionHandler methods *******************/

	/**
	 *  Method that is called by the AMQP library every time it has data
	 *  available that should be sent to RabbitMQ.
	 *  @param  connection  pointer to the main connection object
	 *  @param  data        memory buffer with the data that should be sent to RabbitMQ
	 *  @param  size        size of the buffer
	 */
	void onData(AMQP::Connection *connection, const char *data, size_t size) override;

	/**
	 *  Method that is called by the AMQP library when the login attempt
	 *  succeeded. After this method has been called, the connection is ready
	 *  to use.
	 *  @param  connection      The connection that can now be used
	 */
	void onReady(AMQP::Connection *connection) override;

	 /**
	 *  Method that is called by the AMQP library when a fatal error occurs
	 *  on the connection, for example because data received from RabbitMQ
	 *  could not be recognized.
	 *  @param  connection      The connection on which the error occured
	 *  @param  message         A human readable error message
	 */
	void onError(AMQP::Connection *connection, const char *message) override;

	/**
	 *  Method that is called when the connection was closed. This is the
	 *  counter part of a call to Connection::close() and it confirms that the
	 *  AMQP connection was correctly closed.
	 *
	 *  @param  connection      The connection that was closed and that is now unusable
	 */
	void onClosed(AMQP::Connection *connection) override;

	/**
	 *  Method that is called when the server tries to negotiate a heartbeat
	 *  interval, and that is overridden to get rid of the default implementation
	 *  (which vetoes the suggested heartbeat interval), and accept the interval
	 *  instead.
	 *  @param  connection      The connection on which the error occurred
	 *  @param  interval        The suggested interval in seconds
	 */
	uint16_t onNegotiate(AMQP::Connection *connection, uint16_t interval) override;

	void onHeartbeat(AMQP::Connection *connection) override;
};
