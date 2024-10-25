#pragma once

// #include "time-exceeded-exception.h"

#include <string>
#include <chrono>
#include <vector>
#include <functional>

#undef CALLBACK // this is defined in windows.h and interferes

namespace blfw {

class Timer {
public:
	enum Action {
		EXCEPTION,
		CALLBACK
	};

	template <class UNIT>
	Timer(std::string const& name, std::chrono::duration<UNIT> duration, Action action = EXCEPTION, std::function<void()> cb = nullptr)
		: name_(name)
		, start_(std::chrono::steady_clock::now())
		, durationMicroSec_(std::chrono::duration_cast<std::chrono::microseconds>(duration).count())
		, action_(action)
		, cb_(cb)
		{}

private:
	std::string name_;
	std::chrono::time_point<std::chrono::steady_clock> start_;
	uint64_t durationMicroSec_;
	Action action_;
	std::function<void()> cb_;

	friend class Timers;
};

class Timers {
public:
	/** Add a new Timer to the collection, for the current thread. */
	static void start(Timer t) { timers_.push_back(t); }
	/** Stops and remove all Timer from the collection that match the given name */
	static void stop(std::string const& name);
	/** Stops and removes all timers for the current thread. */
	static void clear();

	/**
	 * Checks all timers belonging to the current thread in the collection.
	 * Any that have expired will trigger their action and then will be removed.
	 */
	static void check();

private:
	static thread_local std::vector<Timer> timers_;
};

} // namespace blfw