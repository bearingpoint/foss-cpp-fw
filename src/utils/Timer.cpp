#include <boglfw/utils/Timer.h>

Timer::Timer(float interval)
	: interval_(interval) {
}

void Timer::setInterval(float value) {
	interval_ = value;
}

void Timer::setLoopMode(bool enable) {
	loop_ = enable;
}

void Timer::start() {
	if (time_ < interval_)
		running_ = true;
}

void Timer::stop() {
	running_ = false;
}

void Timer::reset() {
	time_ = 0;
}

void Timer::restart() {
	reset();
	start();
}

void Timer::update(float dt) {
	if (!running_)
		return;
	time_ += dt;
	if (time_ >= interval_) {
		if (loop_)
			time_ -= interval_;
		else
			running_ = false;
		onTimeout.trigger();
	}
}
