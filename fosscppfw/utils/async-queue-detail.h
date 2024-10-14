#pragma once

#include "semaphore.h"
#include <mutex>
#include <deque>

namespace detail {

template<class T>
class AsyncQueueImpl {
public:
	void push(T && t) {
		std::lock_guard<std::mutex> lock(queueMutex_);
		queue_.push_back(std::forward<T&&>(t));
		semaphore_.notify();
	}

	T pop() {
		semaphore_.wait();
		std::lock_guard<std::mutex> lock(queueMutex_);
		T result = queue_.front();
		queue_.pop_front();
		return result;
	}

	std::vector<T> drain() {
		std::deque<T> replacement;
		{
			std::lock_guard<std::mutex> lock(queueMutex_);
			queue_.swap(replacement);
		}
		return {replacement.begin(), replacement.end()};
	}

private:
	Semaphore semaphore_;
	std::mutex queueMutex_;
	std::deque<T> queue_;
};

} // namespace detail

template<class T>
AsyncQueue<T>::AsyncQueue()
	: pImpl_(new detail::AsyncQueueImpl<T>())
{}

template<class T>
void AsyncQueue<T>::push(T && t) {
	pImpl_->push(std::forward<T&&>(t));
}

template<class T>
T AsyncQueue<T>::pop() {
	return pImpl_->pop();
}

template<class T>
std::vector<T> AsyncQueue<T>::drain() {
	return pImpl_->drain();
}