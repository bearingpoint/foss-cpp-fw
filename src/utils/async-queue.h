#pragma once

#include <vector>

namespace detail {
template<class T>
class AsyncQueueImpl;
}

template<class T>
class AsyncQueue {
public:
	AsyncQueue();
	AsyncQueue(const AsyncQueue&) = delete;

	/**
	 * Pushes a new element to the queue.
	 * This is thread-safe by design.
	 */
	void push(T && t);

	/**
	 * Blocks until an element is available then pops and returns it.
	 * This is thread-safe by design.
	 * Warning: Calling pop() while there are zero publishing threads (that call push()) will block indefinitely.
	 */
	T pop();

	/**
	 * Immediately drains the queue and returns its entire contents.
	 * Does not block. If there is nothing enqueued, it will return an empty vector.
	 * This is thread-safe by design
	*/
	std::vector<T> drain();

private:
	detail::AsyncQueueImpl<T>* pImpl_;
};

#include "async-queue-detail.h"