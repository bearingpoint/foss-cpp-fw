#pragma once

#include <boglfw/net/result.h>
#include <boglfw/net/connection.h>

namespace net {

using listener = unsigned;

// attempts to open a listener on the specified port, for all interfaces.
// returns the listener.
// the provided callback is called asynchronously each time a new client connects or an error is encountered.
// if the result code in the callback is not "OK" then the connection parameter should be ignored since it's not valid.
// the call returns as soon as the listener is established or an error is encountered;
// the listener will run on a separate internal thread
listener startListen(uint16_t port, newConnectionCallback callback);

// turns off a listener and closes the associated port
void stopListen(listener lis);

}
