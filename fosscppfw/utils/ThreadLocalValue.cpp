/*
 * ThreadLocalValue.h
 *
 *  Created on: Apr 13, 2018
 *	  Author: bog
 */

#include "ThreadLocalValue.h"

#ifndef DISABLE_THREAD_LOCAL

namespace ThreadLocalValuePrivate {
	thread_local std::map<void*, vtype> mapThreadValues_;
}

#endif // DISABLE_THREAD_LOCAL