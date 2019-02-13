#pragma once

#include <boglfw/net/result.h>

#include <string>
#include <functional>

namespace net {

using connection = unsigned;
using newConnectionCallback = std::function<void(result, connection)>;

// attempts to connect to a remote host, blocking.
// returns ok and fills outCon on success.
// returns error code on failure.
result connect(std::string host, uint16_t port, connection& outCon);

// starts an asynchronous non-blocking connection attempt to the remote host.
// the provided callback returns a result code (ok on success, other on error) and if it succeeded,
// also returns the connection.
// The connection parameter of the callback is invalid and must be ignored if the result code is not "ok"
void connect_async(std::string host, uint16_t port, newConnectionCallback callback);

// shuts down a connection
void closeConnection(connection con);

// write data to a connection.
// returns ok on success, error code on failure.
// the call is blocking.
result write(connection con, const void* buffer, size_t count);

// read "count" bytes from the connection into buffer.
// returns ok on success, error code on failure.
// the call is blocking.
result read(connection con, void* buffer, size_t bufSize, size_t count);

// Cancels all on-going read/write operations on a connection.
// You can use this for example in case of a timeout
void cancelOperations(connection con);

}
