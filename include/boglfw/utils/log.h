/*
 * log.h
 *
 *  Created on: Nov 14, 2014
 *      Author: bogdan
 */

#ifndef LOG_H_
#define LOG_H_

#define _ENABLE_LOGGING_

#ifdef _ENABLE_LOGGING_

#include <iostream>
#include <deque>
#include <ostream>
#include <string>
#include <mutex>
#include <atomic>

#define LOGPREFIX(PREF) logger_prefix logger_prefix_token(PREF);

#define LOGIMPL(LEVEL, WRITE_PREFIX, X) {\
	if (LEVEL <= logger::instance().getLogLevel()) {\
		if (logger::instance().getLogStream()) {\
			std::lock_guard<std::mutex> lk(logger::getLogMutex());\
			if (WRITE_PREFIX)\
				logger::instance().writeprefix(*logger::instance().getLogStream());\
			*logger::instance().getLogStream() << X;\
		}\
	}\
}

#ifdef DEBUG
#define LOG(X) { LOGIMPL(LOG_LEVEL_INFO, true, X)\
	/* also put the log on stdout in DEBUG mode */\
	if (LOG_LEVEL_INFO <= logger::instance().getLogLevel() && logger::instance().getLogStream() != &std::cout) {\
		std::lock(logger::getLogMutex(), logger::getErrMutex());\
		std::lock_guard<std::mutex> lkL(logger::getLogMutex(), std::adopt_lock);\
		std::lock_guard<std::mutex> lkE(logger::getErrMutex(), std::adopt_lock);\
		logger::instance().writeprefix(std::cout);\
		std::cout << X;\
	}\
}
#else
#define LOG(X) LOGIMPL(LOG_LEVEL_INFO, true, X)
#endif

#define LOGNP(X) LOGIMPL(LOG_LEVEL_INFO, false, X)
#define LOGLN(X) LOG(X << "\n")
#define ERROR(X) {\
	std::lock_guard<std::mutex> lk(logger::getErrMutex());\
	for (auto stream : {&std::cerr, logger::instance().getErrStream()}) {\
		if (!stream)\
			continue;\
		*stream << "[ERROR]";\
		logger::instance().writeprefix(*stream);\
		*stream << X << "\n";\
	}\
}

#else
#define LOGIMPL(LEVEL, WRITE_PREFIX, X)
#define LOG(X)
#define LOGNP(X)
#define LOGLN(X)
#define ERROR(X)
#endif

#define DEBUGLOG(X) LOGIMPL(LOG_LEVEL_DEBUG, true, X)
#define DEBUGLOGNP(X) LOGIMPL(LOG_LEVEL_DEBUG, false, X)
#define DEBUGLOGLN(X) DEBUGLOG(X << "\n")

#ifdef _ENABLE_LOGGING_

enum logLevels {
	LOG_LEVEL_ERROR = 0,
	LOG_LEVEL_INFO,
	LOG_LEVEL_DEBUG
};

class logger {
public:
	void writeprefix(std::ostream &stream);

	// returns old stream
	static std::ostream* setLogStream(std::ostream* newStream) {
		std::lock_guard<std::mutex> lk(logMutex_);
		std::ostream* pOld = instance_.pLogStream_;
		instance_.pLogStream_.store(newStream);
		return pOld;
	}
	// returns old stream
	std::ostream* setAdditionalErrStream(std::ostream* newStream) {
		std::lock_guard<std::mutex> lk(errMutex_);
		std::ostream* pOld = instance_.pErrStream_;
		instance_.pErrStream_.store(newStream);
		return pOld;
	}

	std::ostream* getLogStream() { return pLogStream_; }
	std::ostream* getErrStream() { return pErrStream_; }

	static logger& instance() { return instance_; }

	static std::mutex& getLogMutex() { return logMutex_; }
	static std::mutex& getErrMutex() { return errMutex_; }

	static int getLogLevel() { return logLevel_.load(std::memory_order_acquire); }
	static void setLogLevel(int level) { logLevel_.store(level, std::memory_order_release); }

private:
	static std::atomic<std::ostream*> pLogStream_;
	static std::atomic<std::ostream*> pErrStream_;
	std::deque<std::string> prefix_;
	static thread_local logger& instance_;
	static std::mutex logMutex_;
	static std::mutex errMutex_;
	static std::atomic_int logLevel_;

	void push_prefix(std::string prefix) { prefix_.push_back(prefix); }
	void pop_prefix() { prefix_.pop_back(); }

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
