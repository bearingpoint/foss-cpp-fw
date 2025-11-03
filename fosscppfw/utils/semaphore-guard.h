/*
  SemaphoreGuard - RAII guard for Semaphore

  Ensures notify() is called when leaving scope to avoid leaks on exceptions.
*/
#pragma once

#include "fosscppfw/utils/semaphore.h"

struct SemaphoreGuard {
	Semaphore &sem_;
	explicit SemaphoreGuard(Semaphore &s) : sem_(s) { sem_.wait(); }
	~SemaphoreGuard() { sem_.notify(); }
	SemaphoreGuard(SemaphoreGuard const&) = delete;
	SemaphoreGuard& operator=(SemaphoreGuard const&) = delete;
};
