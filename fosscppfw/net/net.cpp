#include "connection.h"

#include <asio.hpp>

#include <vector>
#include <mutex>
#include <thread>
#include <atomic>

/*
	IMPORTANT LINKS:

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

struct ConnectionInfo {
	tcp::socket* socket;

	explicit ConnectionInfo(tcp::socket* socket) : socket(socket) {}
};

static asio::io_context theIoContext;

static result translateError(const asio::error_code &err);

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
		outCon = new ConnectionInfo(newSocket);
		return result::ok;
	} else {
		outCon = nullptr;
		return translateError(err);
	}
}

void closeConnection(connection con) {
	assert(con && con->socket);
	try {
		con->socket->shutdown(tcp::socket::shutdown_both);
		con->socket->close();
	} catch (std::exception const& e) {
	}
	con->socket = nullptr;
}

size_t bytesAvailable(connection con) {
	assert(con && con->socket);
	return con->socket->available();
}

result write(connection con, const void* buffer, size_t count) {
	assert(con && con->socket);
	asio::error_code err;
	asio::write(*con->socket, asio::buffer(buffer, count), err);
	return translateError(err);
}

result read(connection con, void* buffer, size_t bufSize, size_t count) {
	assert(con && con->socket);
	assert(count <= bufSize);
	asio::error_code err;
	asio::read(*con->socket, asio::buffer(buffer, count), err);
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
