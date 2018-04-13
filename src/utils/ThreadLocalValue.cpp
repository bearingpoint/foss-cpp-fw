/*
 * ThreadLocalValue.h
 *
 *  Created on: Apr 13, 2018
 *      Author: bog
 */

#include <boglfw/utils/ThreadLocalValue.h>

namespace ThreadLocalValuePrivate {
	thread_local std::map<void*, vtype> mapThreadValues_;
}