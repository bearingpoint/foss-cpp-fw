#ifndef __UDP_H__
#define __UDP_H__

#include <boglfw/net/result.h>

#include <string>
#include <functional>

namespace net {

using udpSocket = unsigned;

// create a new UDP socket and configure it for multicast sending on the given multicast address and port
udpSocket createMulticastSendSocket(std::string multicastAddress, unsigned short port);

// create a new UDP socket and configure it for receiving multicast packets
// from the specified interface [listenAddress] and multicast address [multicastGroup]
udpSocket createMulticastReceiveSocket(std::string listenAddress, std::string multicastGroup, unsigned short port);

// creates a simple UDP socket which can be used for writing and reading
udpSocket createUDPSocket(unsigned short port);

// closes a UDP socket
void closeSocket(udpSocket socket);

// write data to a socket.
// returns ok on success, error code on failure.
// the call is blocking.
// if the provided socket is a multicast socket, the write operation will be treated as a multicast, otherwise as unicast
result writeUDP(udpSocket socket, const void* buffer, size_t count);

// read "count" bytes from the socket into buffer.
// returns ok on success, error code on failure.
// the call is blocking.
result readUDP(udpSocket socket, void* buffer, size_t bufSize, size_t count);

} // namespace

#endif //__UDP_H__
