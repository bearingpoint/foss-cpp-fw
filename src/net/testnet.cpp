//#define TEST_NET_ENABLED
#ifdef TEST_NET_ENABLED

#include <boglfw/net/listener.h>
#include <boglfw/net/connection.h>

#include <boglfw/utils/semaphore.h>

#include <stdexcept>
#include <cstring>
#include <iostream>

void testHost(int port);
void testClient(char* host, int port);

void testNet(int argc, char** argv) {
	try {
		if (argc < 3)
			throw std::runtime_error("wrong args");
		if (!strcmp(argv[1], "host"))
			testHost(atoi(argv[2]));
		else if (!strcmp(argv[1], "join")) {
			if (argc != 4)
				throw std::runtime_error("wrong args");
			else
				testClient(argv[2], atoi(argv[3]));
		} else
			throw std::runtime_error("wrong args");
	} catch (std::exception &e) {
		std::cerr << e.what() << "\n";
	}
}

void runChat(net::connection con, bool first) {
	bool myTurn = first;
	while (!std::cin.eof()) {
		if (myTurn) {
			std::cout << "** your turn:\n";
			std::string line;
			std::cin >> line;
			size_t len = line.size()+1;
			auto err = net::write(con, &len, sizeof(len));
			if (err == net::result::ok)
				err = net::write(con, line.c_str(), len);
			if (err != net::result::ok) {
				std::cout << "** FAILED to send message: " << errorString(err) << "\n";
				return;
			}
		} else {
			char buf[1024];
			size_t len;
			std::cout << "    >>> ";
			auto err = net::read(con, &len, sizeof(len), sizeof(len));
			if (err == net::result::ok) {
				size_t recv = 0;
				while (recv < len) {
					size_t chunkSize = std::min(sizeof(buf)-1, len-recv);
					err = net::read(con, buf, sizeof(buf), chunkSize);
					if (err != net::result::ok) {
						std::cout << "** FAILED to receive message: " << errorString(err) << "\n";
						return;
					} else {
						buf[chunkSize] = 0;
						std::cout << buf;
						recv += chunkSize;
					}
				}
				std::cout << "\n";
			} else {
				std::cout << "**FAILED to receive message: " << errorString(err) << "\n";
				return;
			}
		}
		myTurn = !myTurn;
	}
}

void testHost(int port) {
	std::cout << "** Start listening on port " << port << "...\n";
	net::listener listener;
	semaphore clientConnected;
	net::connection clientCon;
	net::startListen((uint16_t)port, listener, [&] (net::result res, net::connection con) {
		if (res != net::result::ok) {
			std::cout << "** Incomming connection failed: " << errorString(res) << "\n";
		} else {
			std::cout << "** Client connected. Waiting for him to say hi...\n";
			clientConnected.notify();
		}
	});
	clientConnected.wait();
	net::stopListen(listener);
	runChat(clientCon, false);
	std::cout << "** Closing connection...\n";
	net::closeConnection(clientCon);
	std::cout << "** Connection closed. EXITING.\n";
}

void testClient(char* host, int port) {
	std::cout << "** Connecting to " << host << ":" << port << " ...\n";
	net::connection con;
	auto res = net::connect(host, port, con);
	if (res != net::result::ok) {
		std::cout << "** FAILED to connect: " << errorString(res) << "\n";
		return;
	} else {
		std::cout << "** CONNECTED. Say hello!\n";
		runChat(con, true);
		std::cout << "** Closing connection...\n";
		net::closeConnection(con);
		std::cout << "** Connection closed. EXITING.\n";
	}
}

#endif
