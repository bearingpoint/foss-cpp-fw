#pragma once

#include "semaphore.h"

class semaphoreAcquire
{
private:
	Semaphore& semaphore_;
public:
	semaphoreAcquire(Semaphore& semaphore):
		semaphore_(semaphore)
	{
		semaphore_.wait();
	}

	~semaphoreAcquire() {
		semaphore_.notify();
	}
};
