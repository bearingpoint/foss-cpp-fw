/*
 * marker.h
 *
 *  Created on: Jul 22, 2016
 *	  Author: bog
 */

#ifndef PERF_MARKER_H_
#define PERF_MARKER_H_

#include "callGraph.h"
#include "frameCapture.h"

#include <chrono>
#include <string>

#ifdef ENABLE_PERF_PROFILING
	#define ENABLE_PERF_MARKERS
#else
	#undef ENABLE_PERF_MARKERS
#endif

#ifdef ENABLE_PERF_MARKERS
	#define COMBINE1(X,Y) X##Y  // helper macro
	#define COMBINE(X,Y) COMBINE1(X,Y)
	#define PERF_MARKER_FUNC perf::Marker COMBINE(funcMarker,__LINE__)(__PRETTY_FUNCTION__)
	#define PERF_MARKER_FUNC_BLOCKED perf::Marker COMBINE(funcMarker,__LINE__)(__PRETTY_FUNCTION__, true)
	#define PERF_MARKER(NAME) perf::Marker COMBINE(perfMarker, __LINE__)(NAME)
	#define PERF_MARKER_BLOCKED(NAME) perf::Marker COMBINE(perfMarker,__LINE__)(NAME, true)
	#define PERF_MARKER_IDLE(NAME) perf::Marker COMBINE(perfMarker,__LINE__)(NAME, false, true)
#else
	#define PERF_MARKER_FUNC
	#define PERF_MARKER_FUNC_BLOCKED
	#define PERF_MARKER(NAME)
	#define PERF_MARKER_BLOCKED(NAME)
	#define PERF_MARKER_IDLE(NAME)
#endif

namespace perf {

inline void setCrtThreadName(std::string name) {
#ifdef ENABLE_PERF_PROFILING
	CallGraph::getCrtThreadInstance().threadName_ = name;
#endif
}

#ifdef ENABLE_PERF_PROFILING
class Marker {
public:
	Marker(const char name[], bool blocked = false, bool idle = false) {
		CallGraph::pushSection(name, blocked);
		start_ = std::chrono::high_resolution_clock::now();
		if (FrameCapture::captureEnabledOnThisThread()) {
			FrameCapture::beginFrame(name, start_, blocked, idle);
		}
	}

	~Marker() {
		auto end = std::chrono::high_resolution_clock::now();
		auto nanosec = std::chrono::nanoseconds(end - start_).count();
		CallGraph::popSection(nanosec);
		if (FrameCapture::captureEnabledOnThisThread()) {
			FrameCapture::endFrame(end);
		}
	}

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

#endif // ENABLE_PERF_PROFILING

} // namespace perf

#endif /* PERF_MARKER_H_ */
