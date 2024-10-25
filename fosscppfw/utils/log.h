/*
 * log.h
 *
 *  Created on: Nov 14, 2014
 *	  Author: bogdan
 */

#ifndef LOG_H_
#define LOG_H_

#define _ENABLE_LOGGING_

#ifdef _ENABLE_LOGGING_

#ifdef DISABLE_THREAD_LOCAL
	#define LOGGER_THREAD_LOCAL
	#define SHARED_LOGGER_INSTANCE true
#else
	#define LOGGER_THREAD_LOCAL thread_local
	#define SHARED_LOGGER_INSTANCE false
#endif

#include "ioModif.h"

#include <iostream>
#include <vector>
#include <ostream>
#include <string>
#include <mutex>
#include <atomic>
#include <memory>
#if SHARED_LOGGER_INSTANCE
#	include <unordered_map>
#	include <thread>
#endif

#define LOGPREFIX(PREF) logger_prefix logger_prefix_token(PREF);

/** Use EM_ON << "text" << EM_OFF to emphasize some text */
#define EM_ON ioModif::SELECTED
#define EM_OFF ioModif::NO_SELECTED

#define LOGIMPL(LEVEL, WRITE_PREFIX, X) {\
	if (LEVEL <= logger::instance().getLogLevel()) {\
		for (auto sinkPtr : {logger::getStdOutSink(), logger::getAddLogSink()}) {\
			if (!sinkPtr)\
				continue;\
			std::lock_guard<std::mutex> sinkLock(*sinkPtr->getMutex());\
			if (WRITE_PREFIX)\
				logger::instance().writeprefix(sinkPtr->getStream());\
			sinkPtr->getStream() << X;\
			if (&sinkPtr->getStream() == &std::cout) \
				sinkPtr->getStream() << ioModif::RESET; \
			sinkPtr->getStream().flush();\
		}\
	}\
}

#define LOG(X, LEVEL) LOGIMPL(LEVEL, true, X)

#define LOGNP(X) LOGIMPL(LOG_LEVEL_INFO, false, X)
#define LOGLN(X) LOG(X << "\n", LOG_LEVEL_INFO)
#define LOGRW(X) { LOGNP("\r"); LOG(X, LOG_LEVEL_INFO) }
#define ERRORLOG(X) {\
	for (auto sinkPtr : {logger::getStdErrSink(), logger::getAddErrSink()}) {\
		if (!sinkPtr)\
			continue;\
		std::lock_guard<std::mutex> sinkLock(*sinkPtr->getMutex());\
		if (&sinkPtr->getStream() == &std::cerr) \
			sinkPtr->getStream() << ioModif::FG_RED;\
		logger::instance().writeprefix(sinkPtr->getStream(), true);\
		sinkPtr->getStream() << X << "\n";\
		if (&sinkPtr->getStream() == &std::cerr) \
			sinkPtr->getStream() << ioModif::RESET;\
		sinkPtr->getStream().flush();\
	}\
}

#else
#define LOGIMPL(LEVEL, WRITE_PREFIX, X)
#define LOG(X)
#define LOGRW(X)
#define LOGNP(X)
#define LOGLN(X)
#define ERRORLOG(X)
#endif

#define DEBUGLOG(X) LOGIMPL(LOG_LEVEL_DEBUG, true, X)
#define DEBUGLOGNP(X) LOGIMPL(LOG_LEVEL_DEBUG, false, X)
#define DEBUGLOGLN(X) DEBUGLOG(X << "\n")

#ifdef _ENABLE_LOGGING_

enum logLevels {
	LOG_LEVEL_ERROR = 0,
	LOG_LEVEL_INFO,
	LOG_LEVEL_VERBOSE,
	LOG_LEVEL_DEBUG
};

class logger_sink {
	friend class logger;
	std::ostream* stream_;
	std::shared_ptr<std::mutex> mutex_;

public:
	logger_sink(std::ostream* stream, std::shared_ptr<std::mutex> mutex)
		: stream_(stream), mutex_(mutex) {}
	std::shared_ptr<std::mutex> getMutex() { return mutex_; }
	std::ostream& getStream() { return *stream_; }
};

class logger {
public:
	void writeprefix(std::ostream &stream, bool error = false);

	// returns old stream
	static std::ostream* setAdditionalLogStream(std::ostream* newStream) {
		auto oldLogSink = instance_.pAddLogSink_.load(std::memory_order_acquire);
		std::ostream* pOld = oldLogSink ? oldLogSink->stream_ : nullptr;
		instance_.pAddLogSink_.store(new logger_sink(newStream, std::make_shared<std::mutex>()), std::memory_order_release);
		return pOld;
	}
	static logger_sink* getStdOutSink() {
		return &stdOutSink_;
	}
	static logger_sink* getAddLogSink() {
		return pAddLogSink_.load(std::memory_order_acquire);
	}

	// returns old stream
	static std::ostream* setAdditionalErrStream(std::ostream* newStream) {
		auto oldErrSink = instance_.pAddErrSink_.load(std::memory_order_acquire);
		std::ostream* pOld = oldErrSink ? oldErrSink->stream_ : nullptr;
		instance_.pAddErrSink_.store(new logger_sink(newStream, std::make_shared<std::mutex>()), std::memory_order_release);
		return pOld;
	}
	static logger_sink* getStdErrSink() {
		return &stdErrSink_;
	}
	static logger_sink* getAddErrSink() {
		return pAddErrSink_.load(std::memory_order_acquire);
	}

	static logger& instance() { return instance_; }

	static int getLogLevel() { return logLevel_.load(std::memory_order_acquire); }
	static void setLogLevel(int level) { logLevel_.store(level, std::memory_order_release); }

private:
	static logger_sink stdOutSink_;
	static logger_sink stdErrSink_;
	static std::atomic<logger_sink*> pAddLogSink_;
	static std::atomic<logger_sink*> pAddErrSink_;
#if SHARED_LOGGER_INSTANCE
	std::unordered_map<std::thread::id, std::vector<std::string>> prefixByTID_;
	std::mutex loggerPrefixMutex_;
#else
	std::vector<std::string> prefix_;
#endif
	static LOGGER_THREAD_LOCAL logger& instance_;
	static std::atomic_int logLevel_;

	void push_prefix(std::string prefix);
	void pop_prefix();

	friend class logger_prefix;
};

class logger_prefix {
public:
	logger_prefix(std::string s) {
		logger::instance_.push_prefix(s);
	}
	~logger_prefix() {
		logger::instance_.pop_prefix();
	}
};

#endif // _ENABLE_LOGGING_

#define NOT_IMPLEMENTED throw std::runtime_error(std::string("Not implemented: ") + __PRETTY_FUNCTION__ + " : " + std::to_string(__LINE__))

#endif /* LOG_H_ */
