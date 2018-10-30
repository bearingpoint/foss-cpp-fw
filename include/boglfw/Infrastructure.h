/*
 * Infrastructure.h
 *
 *  Created on: Jun 23, 2016
 *      Author: bog
 */

#ifndef INFRASTRUCTURE_H_
#define INFRASTRUCTURE_H_

#include "utils/ThreadPool.h"

class Infrastructure {
public:
	// call this before exiting in order to stop the thread pool and free resources
	static void shutDown() { getInst(true); }

	static ThreadPool& getThreadPool() { return getInst(false).threadPool_; }

private:
	Infrastructure();
	static Infrastructure& getInst(bool shuttingDown) {
		static bool initialized = false;
		if (shuttingDown) {
			if (initialized)
				getInst(false).shutDown_();
			return *(Infrastructure*)nullptr;
		} else {
			initialized = true;
			static Infrastructure instance;
			return instance;
		}
	}

	void shutDown_();

	ThreadPool threadPool_;
};


#endif /* INFRASTRUCTURE_H_ */
