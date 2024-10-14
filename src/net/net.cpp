#include "connection.h"

#include <asio.hpp>

#include <vector>
#include <mutex>
#include <thread>
#include <atomic>

#ifdef DISABLE_THREAD_LOCAL
#define OPTIONAL_THREAD_LOCAL
#else
#define OPTIONAL_THREAD_LOCAL thread_local
#endif

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

static OPTIONAL_THREAD_LOCAL asio::io_context theIoContext;
static OPTIONAL_THREAD_LOCAL std::vector<tcp::socket*> connections;
#ifdef DISABLE_THREAD_LOCAL
static std::mutex mtxConnections;
#define LOCK_IF_NO_THREAD_LOCAL() std::lock_guard<std::mutex> lockIfNoThreadLocal(mtxConnections)
#else
#define LOCK_IF_NO_THREAD_LOCAL()
#endif

static result translateError(const asio::error_code &err);

static tcp::socket* getSocket(connection con) {
	LOCK_IF_NO_THREAD_LOCAL();
	assert(con < connections.size() && connections[con] != nullptr);
	return connections[con];
}

result connect(std::string host, uint16_t port, connection& outCon) {
	tcp::resolver resolver(theIoContext);
	tcp::resolver::results_type endpoints;
	try {
		endpoints = resolver.resolve(host, std::to_string(port));
	} catch (std::exception &e) {
		return result(result::err_unknown, e.what());
	}
	tcp::socket* newSocket = new tcp::socket(theIoContext);
	asio::error_code err;
	asio::connect(*newSocket, endpoints, err);
	if (!err) {
		LOCK_IF_NO_THREAD_LOCAL();
		connections.push_back(newSocket);
		outCon = connections.size() - 1;
		return result::ok;
	} else {
		outCon = -1;
		return translateError(err);
	}
}

void closeConnection(connection con) {
	LOCK_IF_NO_THREAD_LOCAL();
	assert(con < connections.size() && connections[con] != nullptr);
	try {
		connections[con]->shutdown(tcp::socket::shutdown_both);
		connections[con]->close();
	} catch (std::exception const& e) {
	}
	delete connections[con];
	connections[con] = nullptr;
}

size_t bytesAvailable(connection con) {
	auto socket = getSocket(con);
	return socket->available();
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
