/*
 * section.h
 *
 *  Created on: Jul 22, 2016
 *	  Author: bog
 */
#pragma once

#ifdef ENABLE_PERF_PROFILING

#include <string>
#include <vector>
#include <memory>
#include <numeric>

#include <cstring>

namespace perf {

class sectionData {
public:
	std::basic_string<char> getName() const { return name_; }
	bool isDeadTime() const { return deadTime_; }
	uint64_t getInclusiveNanosec() const { return nanoseconds_; }
	uint64_t getExclusiveNanosec() const {
		uint64_t acc = std::accumulate(callees_.begin(), callees_.end(), (uint64_t)0,
			[] (auto sum, auto &callee) {
			return sum + callee->nanoseconds_;
		});
		if (acc > nanoseconds_) {
			return 0;
		}
		return nanoseconds_ - acc;
	}
	unsigned getExecutionCount() const { return executionCount_; }
	const std::vector<std::shared_ptr<sectionData>>& getCallees() const { return callees_; }

private:
	friend class CallGraph;

	static std::shared_ptr<sectionData> make_shared(const char name[]) {
		return std::shared_ptr<sectionData>(new sectionData(name));
	}
	static std::unique_ptr<sectionData> make_unique(const char name[]) {
		return std::unique_ptr<sectionData>(new sectionData(name));
	}

	explicit sectionData(const char name[]) {
		strncpy(name_, name, sizeof(name_)/sizeof(name_[0]) - 1);
	}

	uint64_t nanoseconds_ = 0;
	uint64_t executionCount_ = 0;
	char name_[256];
	bool deadTime_ = false;
	std::vector<std::shared_ptr<sectionData>> callees_;
};

}

#endif // ENABLE_PERF_PROFILING
