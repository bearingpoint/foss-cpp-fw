#pragma once

#include <boglfw/utils/Event.h>

class Timer {
public:
	explicit Timer(float interval);

	void setInterval(float seconds);
	// enable this to make the timer restart itself after timeout; this way the timer will provide a 'tick'
	// every interval.
	// disable this to make the timer run only one time.
	void setLoopMode(bool enable);

	// the event is triggered when the timer reaches the designated interval
	Event<void()> onTimeout;

	// starts the timer; has no effect if the timer is already running or the interval was reached.
	void start();
	// stops the timer and does not reset its time. Has no effect if the timer is already stopped.
	void stop();
	// resets the timer to zero
	void reset();
	// starts or restarts the timer from zero if it's already running
	void restart();

	void update(float dt);

private:
	float interval_;
	float time_ = 0;
	bool loop_ = false;
	bool running_ = false;
};
