#pragma once

#include "semaphore.h"

class semaphoreTryAcquire
{
private:
	Semaphore& semaphore_;

public:
	const bool acquired;

	semaphoreTryAcquire(Semaphore& semaphore):
		semaphore_(semaphore),
		acquired(semaphore_.try_wait())
	{}

	~semaphoreTryAcquire() {
		if (acquired) {
			semaphore_.notify();
		}
	}
};
