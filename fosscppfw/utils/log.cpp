/*
 * log.cpp
 *
 *  Created on: Nov 28, 2014
 *	  Author: bogdan
 */

#include "log.h"

#ifdef _ENABLE_LOGGING_

#include <ctime>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <deque>

LOGGER_THREAD_LOCAL logger theInstance;
LOGGER_THREAD_LOCAL logger& logger::instance_ { theInstance };
std::atomic_int logger::logLevel_ { LOG_LEVEL_INFO };

// these two share the same mutex:
logger_sink logger::stdOutSink_ {&std::cout, std::make_shared<std::mutex>()};
logger_sink logger::stdErrSink_ {&std::cerr, logger::stdOutSink_.getMutex()};
std::atomic<logger_sink*> logger::pAddLogSink_ {nullptr};
std::atomic<logger_sink*> logger::pAddErrSink_ {nullptr};

std::string formatCrtDateTime() {
	time_t t = time(0);   // get time now
	struct tm * now = localtime( & t );
	// output date & time in this format: "yyyy-mm-dd hh:mm:ss"
	std::stringstream s;
	s << 1900+now->tm_year << "-";
	s << std::setw(2) << std::setfill('0') << 1 + now->tm_mon << "-";
	s << std::setw(2) << std::setfill('0') << now->tm_mday << " ";
	s << std::setw(2) << std::setfill('0') << now->tm_hour << ":";
	s << std::setw(2) << std::setfill('0') << now->tm_min << ":";
	s << std::setw(2) << std::setfill('0') << now->tm_sec;

	return s.str();
}

void logger::push_prefix(std::string prefix) {
#if SHARED_LOGGER_INSTANCE
	std::lock_guard<std::mutex> lock(loggerPrefixMutex_);
	prefixByTID_[std::this_thread::get_id()].push_back(prefix);
#else
	prefix_.push_back(prefix);
#endif
}
void logger::pop_prefix() {
#if SHARED_LOGGER_INSTANCE
	std::lock_guard<std::mutex> lock(loggerPrefixMutex_);
	prefixByTID_[std::this_thread::get_id()].pop_back();
#else
	prefix_.pop_back();
#endif
}

void logger::writeprefix(std::ostream &stream) {
	// 1. write timestamp
	stream << "{" << formatCrtDateTime() << "} ";

	// 2. write logger name:
#if SHARED_LOGGER_INSTANCE
	std::lock_guard<std::mutex> lock(loggerPrefixMutex_);
	auto &prefix = prefixByTID_[std::this_thread::get_id()];
#else
	auto &prefix = prefix_;
#endif
	if (prefix.size())
		stream << "[";
	for (unsigned i=0, n=prefix.size(); i<n; i++)
		stream << (i==0 ? "" : "::") << prefix[i];
	if (prefix.size())
		stream << "] ";
}

#endif // _ENABLE_LOGGING_
