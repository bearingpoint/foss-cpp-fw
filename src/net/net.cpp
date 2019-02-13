#include <boglfw/net/connection.h>
#include <boglfw/net/listener.h>

#include <boglfw/utils/semaphore.h>

#include <asio.hpp>

#include <vector>
#include <mutex>
#include <thread>
#include <atomic>

namespace net {

using asio::ip::tcp;

static asio::io_context theIoContext;
static std::vector<tcp::socket*> connections;
static std::vector<tcp::socket*> connectionsToDelete;
static std::vector<tcp::acceptor*> listeners;
static std::vector<tcp::acceptor*> listenersToDelete;
static semaphore workAvail;
static std::mutex asyncOpMutex;		// used to synchronize the changes to io_context run thread's state with operations that create or destroy
												// async objects such as connections and listeners

static void checkStart();	// checks if a worker thread is running and if not it starts one for all async operations
static void checkFinish(std::unique_lock<std::mutex> &lk);	// checks if all connections and listeners are closed and if so, shuts down the worker thread
static result translateError(const asio::error_code &err);

static tcp::socket* getSocket(connection con) {
	assert(con < connections.size() && connections[con] != nullptr);
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

void startListen(uint16_t port, listener &outLis, newConnectionCallback callback) {
	tcp::acceptor* acceptor = new tcp::acceptor(theIoContext, tcp::endpoint(tcp::v4(), port));
	std::lock_guard<std::mutex> lk(asyncOpMutex);
	listeners.push_back(acceptor);
	outLis = listeners.size() - 1;
	startListenImpl(acceptor, callback);
	checkStart();
}

void stopListen(listener lis) {
	std::unique_lock<std::mutex> lk(asyncOpMutex);
	assert(lis < listeners.size() && listeners[lis] != nullptr);
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
	resolver.async_resolve(query, [callback] (const asio::error_code &err, tcp::resolver::iterator endpointIter) {
		if (err) {
			callback(translateError(err), -1u);
		} else {
			tcp::socket* newSocket = new tcp::socket(theIoContext);
			asio::async_connect(*newSocket, endpointIter, [newSocket, callback] (const asio::error_code &err, tcp::resolver::iterator endpointIter) {
				if (!err) {
					std::unique_lock<std::mutex> lk(asyncOpMutex);
					connections.push_back(newSocket);
					auto connectionId = connections.size() - 1;
					lk.unlock();
					callback(result::ok, connectionId);
				} else {
					callback(translateError(err), -1u);
				}
			});
			workAvail.notify();
		}
	});
	std::lock_guard<std::mutex> lk(asyncOpMutex);
	checkStart();
	workAvail.notify();
}

void closeConnection(connection con) {
	std::unique_lock<std::mutex> lk(asyncOpMutex);
	assert(con < connections.size() && connections[con] != nullptr);
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
	assert(count <= bufSize);
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

} // namespace
