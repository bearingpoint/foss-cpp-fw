#include "timers.h"
#include "time-exceeded-exception.h"
#include "../utils/strbld.h"

#include <algorithm>

namespace blfw {

thread_local std::vector<Timer> Timers::timers_;

void Timers::stop(std::string const& name) {
	timers_.erase(std::remove_if(timers_.begin(), timers_.end(),
		[name](const Timer& timer) { return timer.name_ == name; }),
		timers_.end()
	);
}

void Timers::check() {
	std::chrono::time_point now = std::chrono::steady_clock::now();
	for (int i=0; i<timers_.size(); i++) {
		if (std::chrono::duration_cast<std::chrono::microseconds>(now - timers_[i].start_).count() >= timers_[i].durationMicroSec_) {
			Timer t(std::move(timers_[i]));
			timers_.erase(timers_.begin() + i);
			--i;
			// timer has expired
			switch (t.action_) {
			case Timer::Action::EXCEPTION:
				throw TimeExceededException("Timeout [" + t.name_ + "]");
				break;
			case Timer::Action::CALLBACK:
				if (t.cb_) {
					t.cb_();
				}
				break;
			}
		}
	}
}

void Timers::clear() {
	timers_.clear();
}

} // namespace blfw