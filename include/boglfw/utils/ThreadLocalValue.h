/*
 * ThreadLocalValue.h
 *
 *  Created on: Apr 13, 2018
 *      Author: bog
 */

#ifndef THREAD_LOCAL_VALUE_H
#define THREAD_LOCAL_VALUE_H

#include <map>

namespace ThreadLocalValuePrivate {
	using vtype=char[12];
	extern thread_local std::map<void*, vtype> mapThreadValues_;
}

template <class C>
class ThreadLocalValue {
public:
	static_assert(sizeof(C) <= sizeof(ThreadLocalValuePrivate::vtype), "Not enough storage space for type!");

	C get() const;
	void set(C value);

	ThreadLocalValue& operator=(C val) {
		set(val);
		return *this;
	}

	operator C() const {
		return get();
	}

	ThreadLocalValue();
	ThreadLocalValue(C initial);
	~ThreadLocalValue();
};

// ------------------------------------ IMPLEMENTATION ----------------------------------------------

template <class C>
ThreadLocalValue<C>::ThreadLocalValue() {
	set({});
}

template <class C>
ThreadLocalValue<C>::ThreadLocalValue(C initial) {
	set(initial);
}

template <class C>
ThreadLocalValue<C>::~ThreadLocalValue() {
	ThreadLocalValuePrivate::mapThreadValues_.erase((void*)this);
}

template <class C>
C ThreadLocalValue<C>::get() const {
	auto &val = ThreadLocalValuePrivate::mapThreadValues_[(void*)this];
	return *((C*)&val);
}

template <class C>
void ThreadLocalValue<C>::set(C value) {
	*((C*)&ThreadLocalValuePrivate::mapThreadValues_[(void*)this]) = value;
}

#endif // THREAD_LOCAL_VALUE_H
