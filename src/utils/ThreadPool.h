/*
 * ThreadPool.h
 *
 *  Created on: Jun 21, 2016
 *	  Author: bog
 */

#ifndef UTILS_THREADPOOL_H_
#define UTILS_THREADPOOL_H_

#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <atomic>
#include <thread>
#include <utility>

//#define DEBUG_THREADPOOL	// to enable debug logs

#ifdef DEBUG_THREADPOOL
#include "log.h"
#endif

class PoolTask;
using PoolTaskHandle = std::shared_ptr<PoolTask>;

class PoolTask {
public:
	void wait();
	bool isFinished() const;

	static PoolTaskHandle empty() {
		return PoolTaskHandle(new PoolTask(true));
	}

	static PoolTaskHandle combine(std::vector<PoolTaskHandle> &&handles) {
		return PoolTaskHandle(new PoolTask(std::move(handles)));
	}

private:
	std::mutex workMutex_;
	std::atomic<bool> started_ { false };
	std::atomic<bool> finished_ { false };
	std::function<void()> workFunc_;

	bool isCombined_ = false;
	std::vector<PoolTaskHandle> parts_;

	friend class ThreadPool;
	PoolTask(decltype(workFunc_) func)
		: workFunc_(func) {
	}

	PoolTask(bool empty)
		: started_(true)
		, finished_(true)
	{ }

	PoolTask(std::vector<PoolTaskHandle> &&parts)
		: started_(true)
		, isCombined_(true)
		, parts_(std::move(parts))
	{ }
};

class ThreadPool {
public:
	ThreadPool(unsigned numberOfThreads, unsigned maxQueueSize);
	~ThreadPool();	// make sure you call stop() before destruction

	void stop(); // waits for all tasks to finish processing, waits for all workers to finish and shuts down the threads in the pool

	void wait(); // waits for all tasks to finish processing

	size_t getTaskCount() const; // returns the number of tasks that are either running or waiting in the queue. May return a non-up-to-date value.

	template<class F, class... Args>
	PoolTaskHandle queueTask(F task, Args... args) {
		std::unique_lock<std::mutex> lk(poolMutex_);
		while (queueBlocked_.load(std::memory_order_acquire) || queuedTasks_.size() >= maxQueueSize_) {
			lk.unlock();
			std::this_thread::yield();
			lk.lock();
		}
#ifdef DEBUG_THREADPOOL
	LOGLN(__FUNCTION__ << " mutex acquired.");
#endif
		checkValidState();
		auto handle = std::shared_ptr<PoolTask>(new PoolTask([=] () mutable { task(args...); }));
		queuedTasks_.push(handle);
		//lk.unlock(); -- TODO unlocking the mutex here causes the notify_one() below to sometimes hang
#ifdef DEBUG_THREADPOOL
	LOGLN(__FUNCTION__ << " mutex unlocked. notifying...");
#endif
		condPendingTask_.notify_one();
#ifdef DEBUG_THREADPOOL
	LOGLN(__FUNCTION__ << " notify_one() returned");
#endif
		return handle;
	}

	unsigned getThreadCount() const { return workers_.size(); }

protected:
	std::queue<PoolTaskHandle> queuedTasks_;
	unsigned maxQueueSize_;
	std::mutex poolMutex_;
	std::condition_variable condPendingTask_;
	std::vector<std::thread> workers_;
	std::atomic<int> runningTaskCount_ {0};
	std::atomic<bool> queueBlocked_ { false };	// pool is waiting for all tasks completion, calls to queueTask are blocked until operation finishes
	std::atomic<bool> stopSignal_ { false };	// signal workers to stop
	std::atomic<bool> stopRequested_ { false };	// stop requested by user
	std::atomic<bool> stopped_ { false };

	void workerFunc();

	void checkValidState();
	void wait_impl(std::unique_lock<std::mutex> &lk);
};



#endif /* UTILS_THREADPOOL_H_ */
