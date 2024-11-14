#pragma once

#include "result.h"

#include <string>
#include <functional>
#include <stdint.h>

namespace net {

struct ConnectionInfo;
using connection = ConnectionInfo*;

// attempts to connect to a remote host, blocking.
// returns ok and fills outCon on success.
// returns error code on failure.
result connect(std::string host, uint16_t port, connection& outCon);

// shuts down a connection
void closeConnection(connection con);

// returns the amount of data that is ready to be read from a connection
size_t bytesAvailable(connection con);

// write data to a connection.
// returns ok on success, error code on failure.
// the call is blocking.
result write(connection con, const void* buffer, size_t count);

// read "count" bytes from the connection into buffer.
// returns ok on success, error code on failure.
// the call is blocking.
result read(connection con, void* buffer, size_t bufSize, size_t count);
}
