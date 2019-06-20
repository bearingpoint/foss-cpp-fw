#include <boglfw/net/connection.h>
#include <boglfw/net/listener.h>
#include <boglfw/net/udp.h>

#include <boglfw/utils/semaphore.h>

#include <asio.hpp>

#include <vector>
#include <mutex>
#include <thread>
#include <atomic>

/*
	IMPORTANT ADDRESSES:

	ASIO multicast examples:
		* sender
			http://think-async.com/Asio/asio-1.12.2/src/examples/cpp11/multicast/sender.cpp
		* receiver
			http://think-async.com/Asio/asio-1.12.2/src/examples/cpp11/multicast/receiver.cpp
	ASIO library documentation:
		http://think-async.com/Asio/asio-1.12.2/doc/
*/

namespace net {

using asio::ip::tcp;
using asio::ip::udp;

struct UDPSocketWrapper {
	udp::endpoint endpoint;
	udp::socket socket;
	bool isMulticast = false;

	UDPSocketWrapper(asio::io_context& ctx, std::string const& address, unsigned short port)
		: endpoint(asio::ip::make_address(address), port)
		, socket(ctx, endpoint.protocol())
		{}
};

static asio::io_context theIoContext;
static std::vector<tcp::socket*> connections;
static std::vector<tcp::socket*> connectionsToDelete;
static std::vector<tcp::acceptor*> listeners;
static std::vector<tcp::acceptor*> listenersToDelete;
static std::vector<UDPSocketWrapper*> udpSockets;
static semaphore workAvail;
static std::mutex asyncOpMutex;		// used to synchronize the changes to io_context run thread's state with operations that create or destroy
												// async objects such as connections and listeners

static void checkStart();	// checks if a worker thread is running and if not it starts one for all async operations
static void checkFinish(std::unique_lock<std::mutex> &lk);	// checks if all connections and listeners are closed and if so, shuts down the worker thread
static result translateError(const asio::error_code &err);

static tcp::socket* getSocket(connection con) {
	assertDbg(con < connections.size() && connections[con] != nullptr);
	std::lock_guard<std::mutex> lk(asyncOpMutex);
	return connections[con];
}

void startListenImpl(tcp::acceptor* acceptor, newConnectionCallback callback) {
	tcp::socket* clientSocket = new tcp::socket(theIoContext);
	acceptor->async_accept(*clientSocket, [acceptor, clientSocket, callback](const asio::error_code& error) {
		if (!error) {
			std::unique_lock<std::mutex> lk(asyncOpMutex);
			connections.push_back(clientSocket);
			auto connectionId = connections.size() - 1;
			lk.unlock();
			callback(result::ok, connectionId);
		} else {
			delete clientSocket;
			callback(translateError(error), -1u);
		}
		// recurse to continue listening for new clients:
		if (acceptor->is_open())	// (only if the operation wasn't canceled meanwhile)
			startListenImpl(acceptor, callback);
	});
	workAvail.notify();
}

listener startListen(uint16_t port, newConnectionCallback callback) {
	tcp::acceptor* acceptor = new tcp::acceptor(theIoContext, tcp::endpoint(tcp::v4(), port));
	std::lock_guard<std::mutex> lk(asyncOpMutex);
	listeners.push_back(acceptor);
	listener ret = listeners.size() - 1;
	startListenImpl(acceptor, callback);
	checkStart();
	return ret;
}

void stopListen(listener lis) {
	std::unique_lock<std::mutex> lk(asyncOpMutex);
	assertDbg(lis < listeners.size() && listeners[lis] != nullptr);
	listeners[lis]->cancel();
	listeners[lis]->close();
	listenersToDelete.push_back(listeners[lis]);
	listeners[lis] = nullptr;
	checkFinish(lk);
}

result connect(std::string host, uint16_t port, connection& outCon) {
	tcp::resolver resolver(theIoContext);
    tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));
    tcp::socket* newSocket = new tcp::socket(theIoContext);
	asio::error_code err;
    asio::connect(*newSocket, endpoints, err);
	if (!err) {
		std::lock_guard<std::mutex> lk(asyncOpMutex);
		connections.push_back(newSocket);
		outCon = connections.size() - 1;
		return result::ok;
	} else {
		outCon = -1;
		return translateError(err);
	}
}

void connect_async(std::string host, uint16_t port, newConnectionCallback callback) {
	tcp::resolver resolver(theIoContext);
    tcp::resolver::query query(host, std::to_string(port));
	tcp::socket* newSocket = new tcp::socket(theIoContext);
	std::unique_lock<std::mutex> lk(asyncOpMutex);
	connections.push_back(newSocket);
	auto connectionId = connections.size() - 1;
	lk.unlock();
	resolver.async_resolve(query, [callback, newSocket, connectionId] (const asio::error_code &err, tcp::resolver::iterator endpointIter) {
		if (err) {
			std::unique_lock<std::mutex> lk(asyncOpMutex);
			delete newSocket;
			connections[connectionId] = nullptr;
			checkFinish(lk);
			lk.unlock();
			callback(translateError(err), -1u);
		} else {
			asio::async_connect(*newSocket, endpointIter, [newSocket, connectionId, callback] (const asio::error_code &err, tcp::resolver::iterator endpointIter) {
				if (!err) {
					callback(result::ok, connectionId);
				} else {
					std::unique_lock<std::mutex> lk(asyncOpMutex);
					delete newSocket;
					connections[connectionId] = nullptr;
					checkFinish(lk);
					lk.unlock();
					callback(translateError(err), -1u);
				}
			});
			workAvail.notify();
		}
	});
	lk.lock();
	checkStart();
	workAvail.notify();
}

void closeConnection(connection con) {
	std::unique_lock<std::mutex> lk(asyncOpMutex);
	assertDbg(con < connections.size() && connections[con] != nullptr);
	connections[con]->shutdown(tcp::socket::shutdown_both);
	connections[con]->close();
	connectionsToDelete.push_back(connections[con]);
	connections[con] = nullptr;
	checkFinish(lk);
}

result write(connection con, const void* buffer, size_t count) {
	auto socket = getSocket(con);
	asio::error_code err;
	asio::write(*socket, asio::buffer(buffer, count), err);
	return translateError(err);
}

result read(connection con, void* buffer, size_t bufSize, size_t count) {
	assertDbg(count <= bufSize);
	auto socket = getSocket(con);
	asio::error_code err;
	asio::read(*socket, asio::buffer(buffer, count), err);
	return translateError(err);
}

void cancelOperations(connection con) {
	auto socket = getSocket(con);
	socket->cancel();
}

static std::atomic<bool> isContextThreadRunning { false };
static std::atomic<bool> signalContextThreadExit { false };
std::thread contextThread;

static void ioContextThread() {
	while(!signalContextThreadExit.load(std::memory_order_acquire)) {
		workAvail.wait();
		if (signalContextThreadExit.load(std::memory_order_acquire))
			break;

		// here we do the asio work
		theIoContext.run();
		theIoContext.restart();
	}
}


static void checkStart() {
	// we are currently under asyncOpMutex lock by the caller
	if (!isContextThreadRunning.load(std::memory_order_acquire)) {
		unsigned nObjects = 0;
		for (auto &c : connections)
			nObjects += c != nullptr ? 1 : 0;
		for (auto &c : listeners)
			nObjects += c != nullptr ? 1 : 0;
		// we only start the context thread if active objects were found
		if (nObjects > 0) {
			isContextThreadRunning.store(true, std::memory_order_release);
			contextThread = std::thread(&ioContextThread);
		}
	}
}

static void checkFinish(std::unique_lock<std::mutex> &lk) {
	// we are currently under asyncOpMutex lock by the caller
	// count how many live connections/listeners we have:
	unsigned nObjects = 0;
	for (auto &c : connections)
		nObjects += c != nullptr ? 1 : 0;
	for (auto &c : listeners)
		nObjects += c != nullptr ? 1 : 0;
	// if no more objects, we stop the context thread:
	if (nObjects == 0)
	{
		signalContextThreadExit.store(true, std::memory_order_release);
		workAvail.notify();
		lk.unlock();
		contextThread.join();
		contextThread = {};
		lk.lock();
		// signal the thread is done
		isContextThreadRunning.store(false, std::memory_order_release);

		// delete any connections that are done
		for (auto c : connectionsToDelete)
			delete c;
		connectionsToDelete.clear();

		// delete any listeners that are done
		for (auto l : listenersToDelete)
			delete l;
		listenersToDelete.clear();

		// some async operations may have been queued during the time we waited in the .join()
		// but they didn't have a chance to start the thread because isContextThreadRunning wasn't reset until we re-acquired the lock
		checkStart();
	}
}

static result translateError(const asio::error_code &err) {
	if (!err)
		return result::ok;
	else {
		result::result_code code = result::ok;
		switch(err.value()) {
			case asio::error::address_in_use:
				code = result::err_portInUse;
				break;
			case asio::error::connection_refused:
				code = result::err_refused;
				break;
			case asio::error::connection_aborted:
				code = result::err_aborted;
				break;
			case asio::error::connection_reset:
				code = result::err_aborted;
				break;
			case asio::error::host_not_found:
				code = result::err_unreachable;
				break;
			case asio::error::host_unreachable:
				code = result::err_unreachable;
				break;
			case asio::error::timed_out:
				code = result::err_timeout;
				break;
			default:
				code = result::err_unknown;
		}
		return {code, err.message()};
	}
}

// create a new UDP socket and configure it for multicast sending on the given multicast address and port
udpSocket createMulticastSendSocket(std::string multicastAddress, unsigned short port) {
	UDPSocketWrapper* newSock = new UDPSocketWrapper(theIoContext, multicastAddress, port);
	newSock->isMulticast = true;
	//newSock->socket.set_option(asio::ip::multicast::enable_loopback(true));
	//newSock->socket.set_option(asio::socket_base::broadcast(true));

	std::lock_guard<std::mutex> lk(asyncOpMutex);
	udpSockets.push_back(newSock);
	return udpSockets.size() - 1;
}

// create a new UDP socket and configure it for receiving multicast packets
// from the specified interface [listenAddress] and multicast address [multicastGroup]
udpSocket createMulticastReceiveSocket(std::string listenAddress, std::string multicastGroup, unsigned short port) {
	UDPSocketWrapper* newSock = new UDPSocketWrapper(theIoContext, listenAddress, port);
	newSock->isMulticast = true;
	//newSock->socket.open(newSock->endpoint.protocol());
	newSock->socket.set_option(asio::ip::udp::socket::reuse_address(true));
	newSock->socket.bind(newSock->endpoint);
	newSock->socket.set_option(asio::ip::multicast::join_group(asio::ip::make_address(multicastGroup)));

	std::lock_guard<std::mutex> lk(asyncOpMutex);
	udpSockets.push_back(newSock);
	return udpSockets.size() - 1;
}

// creates a simple UDP socket which can be used for writing and reading
udpSocket createUDPSocket(unsigned short port) {
	return -1;
}

// closes a UDP socket
void closeSocket(udpSocket socket) {
	std::lock_guard<std::mutex> lk(asyncOpMutex);
	assertDbg(socket < udpSockets.size() && udpSockets[socket] != nullptr);
	udpSockets[socket]->socket.close();
	delete udpSockets[socket];
	udpSockets[socket] = nullptr;
}

// write data to a socket.
// returns ok on success, error code on failure.
// the call is blocking.
// if the provided socket is a multicast socket, the write operation will be treated as a multicast, otherwise as unicas
result writeUDP(udpSocket socket, const void* buffer, size_t count) {
	std::lock_guard<std::mutex> lk(asyncOpMutex);
	assertDbg(socket < udpSockets.size() && udpSockets[socket] != nullptr);
	if (udpSockets[socket]->isMulticast) {
		auto nSent = udpSockets[socket]->socket.send_to(asio::buffer(buffer, count), udpSockets[socket]->endpoint);
		if (nSent == count)
			return result::ok;
		else
			return result(result::err_unknown, "Incomplete data sent");
	} else {
		// ...
	}
	return result::ok;
}

// read from the socket into buffer and populate [out_count] with the number of bytes read
// and [out_sender] with the endpoint of the sender.
// returns ok on success, error code on failure.
// the call is blocking.
result readUDP(udpSocket socket, void* buffer, size_t bufSize,  size_t &out_count, endpointInfo& out_sender) {
	asio::ip::udp::socket* pSocket = nullptr;
	{
		std::lock_guard<std::mutex> lk(asyncOpMutex);
		assertDbg(socket < udpSockets.size() && udpSockets[socket] != nullptr);
		pSocket = &udpSockets[socket]->socket;
	}
	asio::ip::udp::endpoint senderEndpoint;
	out_count = pSocket->receive_from(asio::buffer(buffer, bufSize), senderEndpoint);
	out_sender.address = senderEndpoint.address().to_string();
	return result::ok;
}

void readUDPAsync(udpSocket socket, void* buffer, size_t bufSize, udpReceiveCallback cb) {
	asio::ip::udp::socket* pSocket = nullptr;
	{
		std::lock_guard<std::mutex> lk(asyncOpMutex);
		assertDbg(socket < udpSockets.size() && udpSockets[socket] != nullptr);
		pSocket = &udpSockets[socket]->socket;
	}
	asio::ip::udp::endpoint *senderEndpoint = new asio::ip::udp::endpoint();
	pSocket->async_receive_from(asio::buffer(buffer, bufSize), *senderEndpoint,
		[cb, senderEndpoint](const asio::error_code& error, size_t bytes_transferred) {
			cb(translateError(error), bytes_transferred, endpointInfo{senderEndpoint->address().to_string()});
			delete senderEndpoint;
		});
	std::lock_guard<std::mutex> lk(asyncOpMutex);
	checkStart();
	workAvail.notify();
}

} // namespace
