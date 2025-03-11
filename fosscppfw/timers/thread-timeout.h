#pragma once

#include <chrono>
#include <atomic>

/** 
	A thread-safe class to determine if a timeout occurred. 
	A timeout is considered "expired" if an interval longer than the specified "milliseconds" value on the constructor
	has elapsed since the object's start() method has been called, and a corresponding call to reset() has not been made until now.
	After thre reset() method is called, the object's expired() method will keep returning false until start() is called again and from that
	point on the interval elapsed becomes longer than the one specified.
*/
class ThreadTimeout {
public:
	ThreadTimeout(int milliseconds)
		: timeoutMs_(milliseconds) {}

	void start() {
		startTime_.store(std::chrono::system_clock::now(), std::memory_order_release);
		running_.store(true, std::memory_order_release);
	}

	void reset() {
		running_.store(false, std::memory_order_release);
	}

	bool expired() const {
		if (!running_.load(std::memory_order_acquire)) {
			return false;
		}
		const auto diff = std::chrono::system_clock::now() - startTime_.load(std::memory_order_acquire);
		return std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() >= timeoutMs_;
	}

private:
	const unsigned long timeoutMs_;
	std::atomic<bool> running_{ false };
	std::atomic<std::chrono::time_point<std::chrono::system_clock>> startTime_;
};