#include "json.h"
#include <algorithm>

#ifdef ENABLE_PERF_PROFILING

namespace perf {
namespace json {

nlohmann::json callFrame(perf::sectionData const& s) {
	nlohmann::json frame;
	frame["name"] = s.getName();
	frame["calls"] = s.getExecutionCount();
	frame["inclusive"] = { {"nanoseconds", s.getInclusiveNanosec()} };
	frame["exclusive"] = { {"nanoseconds", s.getExclusiveNanosec()} };
	frame["blocked"] = s.isDeadTime();
	return frame;
}

nlohmann::json topHits(std::vector<perf::sectionData> data) {
	std::sort(data.begin(), data.end(), [](auto &x, auto &y) {
		return x.getInclusiveNanosec() > y.getInclusiveNanosec();
	});
	nlohmann::json result;
	const size_t maxHits = 6;
	for (unsigned i = 0; i < std::min(maxHits, data.size()); i++) {
		result.push_back(callFrame(data[i]));
	}
	return result;
}

nlohmann::json callTree(std::vector<std::shared_ptr<perf::sectionData>> t) {
	std::sort(t.begin(), t.end(), [](auto &x, auto &y) {
		return x->getInclusiveNanosec() > y->getInclusiveNanosec();
	});
	nlohmann::json result;
	for (auto &s : t) {
		result.push_back(callFrame(*s));
		result.back()["children"] = callTree(s->getCallees());
	}
	return result;
}

nlohmann::json sequenceCapture(std::vector<perf::FrameCapture::frameData> frames) {
	auto referenceTime = frames.front().startTime_;
	// convert any time point into relative amount of nanoseconds since start of frame
	auto deltaTimeMicro = [] (decltype(frames[0].startTime_) &pt, decltype(frames[0].startTime_) &ref) -> int64_t {
		return std::chrono::nanoseconds(pt - ref).count() / 1000;
	};
	// compute metrics:
	auto lastFrame = std::max_element(frames.begin(), frames.end(), [] (
		perf::FrameCapture::frameData const& d1, perf::FrameCapture::frameData const& d2
	) {
		return d1.endTime_ < d2.endTime_;
	});
	int64_t timeSpanUs = deltaTimeMicro(lastFrame->endTime_, referenceTime);

	struct ThreadData {
		nlohmann::json rootFrame;
		std::vector<nlohmann::json*> frameStack;
	};
	std::unordered_map<std::string, ThreadData> threadData;

	for (auto &frame : frames) {
		nlohmann::json frameJson;
		frameJson["tag"] = frame.name_;
		frameJson["startTimeSeq"] = nlohmann::json { {"microseconds", deltaTimeMicro(frame.startTime_, referenceTime)} };
		frameJson["duration"] = nlohmann::json { {"microseconds", deltaTimeMicro(frame.endTime_, frame.startTime_)} };
		frameJson["userData"] = nullptr;
		frameJson["blocked"] = frame.deadTime_;
		frameJson["idle"] = frame.idleTime_;
		frameJson["children"] = nlohmann::json::array();

		ThreadData &thread = threadData[perf::FrameCapture::getThreadNameForIndex(frame.threadIndex_)];
		nlohmann::json *pLastFrame = thread.frameStack.empty() ? nullptr : thread.frameStack.back();
		while (pLastFrame &&
			(*pLastFrame)["startTimeSeq"]["microseconds"].get<int64_t>()
				+ (*pLastFrame)["duration"]["microseconds"].get<int64_t>()
				< frameJson["startTimeSeq"]["microseconds"].get<int64_t>()
		) {
			// previous frame ends before this one starts
			thread.frameStack.pop_back();
			pLastFrame = thread.frameStack.empty() ? nullptr : thread.frameStack.back();
		}

		if (thread.frameStack.empty()) {
			// root frame has not been created yet
			thread.rootFrame["tag"] = perf::FrameCapture::getThreadNameForIndex(frame.threadIndex_);
			thread.rootFrame["startTimeRelative"] = nlohmann::json { {"microseconds", 0} };
			thread.rootFrame["startTimeSeq"] = nlohmann::json { {"microseconds", 0} };
			thread.rootFrame["duration"] = nlohmann::json { {"microseconds", timeSpanUs} };
			thread.rootFrame["userData"] = nullptr;
			thread.rootFrame["blocked"] = false;
			thread.rootFrame["idle"] = false;
			thread.rootFrame["children"] = nlohmann::json::array();
			thread.frameStack.push_back(&thread.rootFrame);
		}

		nlohmann::json &parentFrame = *thread.frameStack.back();
		frameJson["startTimeRelative"] = nlohmann::json { {"microseconds",
			frameJson["startTimeSeq"]["microseconds"].get<int64_t>()
				- parentFrame["startTimeSeq"]["microseconds"].get<int64_t>()
		} };
		parentFrame["children"].push_back(frameJson);
		thread.frameStack.push_back(&parentFrame["children"].back());
	}
	perf::FrameCapture::cleanup();

	nlohmann::json result;
	result["startTime"] = referenceTime.time_since_epoch().count();
	result["duration"] = nlohmann::json { {"seconds", timeSpanUs / 1.e6f} };
	result["rootFrames"] = nlohmann::json::array();
	for (auto &thread : threadData) {
		result["rootFrames"].push_back(thread.second.rootFrame);
	}
	return result;
}

} // namespace json
} // namespace perf

#endif // ENABLE_PERF_PROFILING