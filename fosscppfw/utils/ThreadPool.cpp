/*
 * ThreadPool.cpp
 *
 *  Created on: Jun 21, 2016
 *	  Author: bog
 */

#include "ThreadPool.h"
#include "../perf/marker.h"

#include <cassert>
#include <numeric>
#include <stdexcept>

ThreadPool::ThreadPool(unsigned numberOfThreads, unsigned maxQueueSize)
	: maxQueueSize_(maxQueueSize)
{
#ifdef DEBUG_THREADPOOL
	LOGLN(__FUNCTION__);
#endif
	for (unsigned i=0; i<numberOfThreads; i++)
		workers_.push_back(std::thread(std::bind(&ThreadPool::workerFunc, this)));
#ifdef DEBUG_THREADPOOL
	LOGLN(__FUNCTION__ << " finished.");
#endif
}
ThreadPool::~ThreadPool() {
	assert(stopped_ && "Thread pool has not been stopped before destruction!");
}

void ThreadPool::wait_impl(std::unique_lock<std::mutex> &poolLk) {
	// wait for all tasks to be processed
	checkValidState();
	queueBlocked_.store(true);
	while (getTaskCount() > 0) {
		poolLk.unlock();
		std::this_thread::yield();
		poolLk.lock();
	}
	queueBlocked_.store(false);
}

void ThreadPool::wait() {
	std::unique_lock<std::mutex> poolLk(poolMutex_);
	wait_impl(poolLk);
}

void ThreadPool::stop() {
#ifdef DEBUG_THREADPOOL
	LOGLN(__FUNCTION__);
#endif
	std::unique_lock<std::mutex> poolLk(poolMutex_);
	wait_impl(poolLk);
	// wait for all workers to finish and shuts down the threads in the pool
	stopSignal_.store(true);
	poolLk.unlock();
	condPendingTask_.notify_all();
	for (auto &t : workers_)
		t.join();
	stopped_.store(true);
}

void ThreadPool::workerFunc() {
	perf::setCrtThreadName("ThreadPoolWorker");
#ifdef DEBUG_THREADPOOL
	LOGLN(__FUNCTION__ << " begin");
#endif
	while (!stopSignal_) {
		PoolTaskHandle task(nullptr);
		std::unique_lock<std::mutex> lk(poolMutex_);
		auto pred = [this] { return stopSignal_ || !!!queuedTasks_.empty(); };
		if (!pred()) {
#ifdef DEBUG_THREADPOOL
	LOGLN(__FUNCTION__ << " wait for work...");
#endif
			condPendingTask_.wait(lk, pred);
		}
#ifdef DEBUG_THREADPOOL
	LOGLN(__FUNCTION__ << " woke up.");
#endif
		if (stopSignal_)
			return;
		assert(!!!queuedTasks_.empty());
		task = queuedTasks_.front();
		queuedTasks_.pop();
		lk.unlock();

		runningTaskCount_.fetch_add(1, std::memory_order_release);
		std::lock_guard<std::mutex> workLk(task->workMutex_);
		task->started_.store(true);
		// do work...
		do {
			task->workFunc_();
		} while (0);
		task->finished_.store(true);
		runningTaskCount_.fetch_sub(1, std::memory_order_release);
#ifdef DEBUG_THREADPOOL
	LOGLN(__FUNCTION__ << " finished work.");
#endif
	}
}

bool PoolTask::isFinished() const {
	if (isCombined_) {
		return std::accumulate(parts_.begin(), parts_.end(), true, [](bool finished, auto const& part) {
			return finished && part->isFinished();
		});
	} else {
		return finished_.load(std::memory_order::memory_order_acquire);
	}
}

void PoolTask::wait() {
#ifdef DEBUG_THREADPOOL
	LOGLN(__FUNCTION__ << " waiting for task...");
#endif
	{
		while (!started_)
			std::this_thread::yield();
	}
#ifdef DEBUG_THREADPOOL
	LOGLN(__FUNCTION__ << " task is started.");
#endif
	if (isCombined_) {
		while (!isFinished())
			std::this_thread::yield();
	} else {
		std::lock_guard<std::mutex> lk(workMutex_);
		assert(finished_);
#ifdef DEBUG_THREADPOOL
		LOGLN(__FUNCTION__ << " task is finished.");
#endif
	}
}

void ThreadPool::checkValidState() {
	if (stopRequested_)
		throw std::runtime_error("Invalid operation on thread pool (pool is stopping)");
}

size_t ThreadPool::getTaskCount() const {
	return queuedTasks_.size() + runningTaskCount_.load(std::memory_order_acquire);
}