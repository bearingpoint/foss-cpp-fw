/*
 * Event.h
 *
 *  Created on: Jan 20, 2015
 *      Author: bog
 */

#ifndef EVENT_H_
#define EVENT_H_

#include "assert.h"
#include <functional>
#include <vector>
#include <type_traits>
#include <utility>

template <class T, bool>
struct is_invokable_s : std::false_type {};

template <class T>
struct is_invokable_s<T, true> : std::true_type {};

template <class T>
struct is_invokable :
is_invokable_s<T, std::is_constructible<std::function<T>, decltype(nullptr)>::value>
{};

template <class T>
constexpr bool is_invokable_v = is_invokable<T>::value;


template <class T, std::enable_if_t<is_invokable_v<T>, int> = 0>
class Event {
public:
	using handler_type = std::function<T>;

	Event() = default;

	Event(Event &&e)
		: callbackList_(std::move(e.callbackList_))
		, pForwarded_(e.pForwarded_) {}
	Event(Event const&) = default;
	Event& operator=(Event const&) = default;
	Event& operator=(Event &&) = default;

	int add(handler_type fn) {
		callbackList_.push_back(fn);
		return callbackList_.size() - 1;
	}

	void remove(int handle) {
		assertDbg(handle >= 0 && (unsigned)handle < callbackList_.size());
		callbackList_[handle] = nullptr;
	}

	// Forward this event to another; all triggerings of this event will also trigger the forwarded event.
	void forward(Event<T> &forward) {
		pForwarded_ = &forward;
	}

	void clear() {
		callbackList_.clear();
	}

	void trigger() {
		if (pForwarded_)
			pForwarded_->trigger();
		for (auto &c : callbackList_)
			if (c)
				c();
	}

	template<class... argTypes>
	void trigger(argTypes... argList) {
		if (pForwarded_)
			pForwarded_->trigger(argList...);
		for (auto &c : callbackList_)
			if (c)
				c(argList...);
	}

protected:
	std::vector<handler_type> callbackList_;
	Event<T> *pForwarded_ = nullptr;
};

#endif /* EVENT_H_ */
