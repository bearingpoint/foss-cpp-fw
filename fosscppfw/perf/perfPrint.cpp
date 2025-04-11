/*
 * perfPrint.cpp
 *
 *  Created on: Dec 30, 2016
 *	  Author: bog
 */

#ifdef ENABLE_PERF_PROFILING

#include "./perfPrint.h"
#include "../utils/ioModif.h"

#include <thread>
#include <chrono>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <map>
#include <memory>
#include <stdint.h>

#ifndef WIN32_MSVC
#include <unistd.h>
#endif
#ifndef __WIN32__
#include <sys/ioctl.h>
#endif

std::string formatTime(uint64_t val, int unitIndex = 0, uint64_t fraction = 0) {
	// first is the unit, second is the multiple required to reach next unit
	static constexpr std::pair<const char*, int> units[] = {
		{" ns", 1000},
		{" us", 1000},
		{" ms", 1000},
		{" s", 60},
		{" m", 60},
		{" h", INT64_MAX}
	};
	static constexpr int unitsCount = sizeof(units) / sizeof(units[0]);
	std::stringstream str;
	if (val < units[unitIndex].second || unitIndex == unitsCount - 1) {
		// either small value or we're at the highest unit
		str << val;
		if (fraction) {
			if (units[unitIndex - 1].second == 1000) {
				str << "." << fraction;
			} else {
				str << ":" << fraction;
			}
		}
		str << units[unitIndex].first;
	} else {
		return formatTime(val / units[unitIndex].second, unitIndex + 1, val % units[unitIndex].second);
	}
	return str.str();
}


void printCallFrame(perf::sectionData const& s, bool flatMode/*=false*/, std::ostream &os) {
	bool missingInfo = s.getExclusiveNanosec() > s.getInclusiveNanosec() / 10; // more than 10% unknown
	if (s.getInclusiveNanosec() < 1e9)
		missingInfo = false; // this is not significant

	if (s.isDeadTime())
		os << ioModif::BG_RGB(80,60,60) << ioModif::DARK;

	os << ioModif::BOLD << ioModif::FG_LIGHT_YELLOW << s.getName() << ioModif::FG_DEFAULT << ioModif::NO_BOLD;
	if (s.isDeadTime())
		os << ioModif::BG_RGB(80,60,60) << ioModif::DARK;
	os << "    {"
		<< "calls " << s.getExecutionCount() << " | "
		<< "inc " << ioModif::FG_LIGHT_GREEN << formatTime(s.getInclusiveNanosec()) << ioModif::FG_DEFAULT << " | ";
	if (!flatMode)
		os << "exc " << (missingInfo ? ioModif::FG_RED : ioModif::FG_DEFAULT)
				<< formatTime(s.getExclusiveNanosec()) << ioModif::FG_DEFAULT << " | ";
	if(s.getExecutionCount() > 0) {
		os << "avg-inc " << formatTime(s.getInclusiveNanosec() / s.getExecutionCount()) << " | ";
		if (!flatMode)
			os << "avg-exc " << formatTime(s.getExclusiveNanosec() / s.getExecutionCount());
	}
	os << "}" << ioModif::RESET;
}

void printCallTree(std::vector<std::shared_ptr<perf::sectionData>> t, int level, std::ostream &os) {
	std::sort(t.begin(), t.end(), [](auto &x, auto &y) {
		return x->getInclusiveNanosec() > y->getInclusiveNanosec();
	});
	const auto tab = "	";
	for (auto &s : t) {
		for (int i=0; i<level; i++) {
			os <<"|" << tab;
		}
		os << "|--";
		printCallFrame(*s, false, os);
		os << "\n";
		printCallTree(s->getCallees(), level+1, os);
	}
}

void printTopHits(std::vector<perf::sectionData> data, std::ostream &os) {
	std::sort(data.begin(), data.end(), [](auto &x, auto &y) {
		return x.getInclusiveNanosec() > y.getInclusiveNanosec();
	});
	const size_t maxHits = 6;
	for (unsigned i=0; i<std::min(maxHits, data.size()); i++) {
		os << i << ": ";
		printCallFrame(data[i], true, os);
		os << "\n";
	}
}

void dumpFrameCaptureData(std::vector<perf::FrameCapture::frameData> data, std::ostream &os) {
	auto referenceTime = data.front().startTime_;
	// convert any time point into relative amount of nanoseconds since start of frame
	auto relativeNano = [referenceTime] (decltype(data[0].startTime_) &pt) -> int64_t {
		return std::chrono::nanoseconds(pt - referenceTime).count();
	};
	for (auto &f : data) {
		os << "FRAME " << f.name_ << "\n\t" << "thread: " << f.threadIndex_ << "\tstart: "
				<< relativeNano(f.startTime_)/1000 << "\tend: " << relativeNano(f.endTime_)/1000 << "\n";
	}
}

void printFrameCaptureStatistics(std::vector<perf::FrameCapture::frameData> data, std::ostream &os) {
	os << "============= FRAME CAPTURE STATS ================\n";
	os << "Total Frame time: " << formatTime((data.back().endTime_-data.front().startTime_).count()) << "\n";
	os << data.size() << " frames total\n";
	std::map<int, int> framesPerThread;
	for (auto &f : data)
		framesPerThread[f.threadIndex_]++;
	os << "Frames distribution:\n";
	for (auto &p : framesPerThread)
		os << "\tThread " << p.first << ": " << p.second << " frames\n";
	os << "Average frames per thread: " << std::accumulate(framesPerThread.begin(), framesPerThread.end(), 0, [] (int x, auto &p) {
		return x + p.second;
	}) / framesPerThread.size() << "\n";
}

void printFrameCaptureData(std::vector<perf::FrameCapture::frameData> data, std::ostream &os) {
	//dumpFrameCaptureData(data);
	printFrameCaptureStatistics(data, os);
	auto referenceTime = data.front().startTime_;
	// convert any time point into relative amount of nanoseconds since start of frame
	auto relativeNano = [referenceTime] (decltype(data[0].startTime_) &pt) -> int64_t {
		return std::chrono::nanoseconds(pt - referenceTime).count();
	};
	// compute metrics:
	auto lastFrame = std::max_element(data.begin(), data.end(), [] (perf::FrameCapture::frameData const& d1, perf::FrameCapture::frameData const& d2) {
		return d1.endTime_ < d2.endTime_;
	});
	int64_t timeSpan = relativeNano(lastFrame->endTime_);
#ifndef __WIN32__
	struct winsize sz;
	ioctl(STDOUT_FILENO,TIOCGWINSZ,&sz);
	int lineWidth = sz.ws_col - 10;
#else
	int lineWidth = 70;
#endif
	if (lineWidth <= 0)
		lineWidth = 80; // asume default
	double cellsPerNanosec = (lineWidth-1.0) / timeSpan;

	// build visual representation
	struct threadData {
		std::vector<std::unique_ptr<std::stringstream>> str;
		std::vector<int> strOffs;
		std::map<std::string, unsigned> legend;
		std::vector<unsigned> callsEndTime;

		threadData() = default;
		threadData(threadData &&t) = default;
	};
	struct rgbColor {
		int r, g, b;
		operator ioModif::BG_RGB() {
			return ioModif::BG_RGB(r, g, b);
		}
		operator ioModif::FG_RGB() {
			return ioModif::FG_RGB(r*1.5, g*1.5, b*1.5);
		}
	} colors[] = {
		{50,50,150},
		{0,150,0},
		{150,150,0},
		{150,0,0},
		{150,0,150},
	};
	auto colorsCount = sizeof(colors)/sizeof(colors[0]);
	std::vector<threadData> threads;
	for (auto &f : data) {
		while (threads.size() <= f.threadIndex_)
			threads.push_back(threadData());
		threadData &td = threads[f.threadIndex_];
		// see if this frame appeared before in this thread:
		if (td.legend.find(f.name_) == td.legend.end())
			td.legend.insert({f.name_, td.legend.size()});
		// check if need to pop a stack level
		while (td.callsEndTime.size() >= 2 && *(td.callsEndTime.end()-2) < relativeNano(f.endTime_))
			td.callsEndTime.pop_back();
		// check if this is a new level on the stack
		if (td.callsEndTime.empty() || relativeNano(f.endTime_) < td.callsEndTime.back())
			td.callsEndTime.push_back(relativeNano(f.endTime_));
		else {
			td.callsEndTime.back() = relativeNano(f.endTime_);
		}
		int frameID = td.legend[f.name_];
		while (td.str.size() < td.callsEndTime.size()) {
			td.str.push_back(std::make_unique<std::stringstream>());
			td.strOffs.push_back(0);
		}
		auto& crtStr = *td.str[td.callsEndTime.size()-1];
		auto& crtStrOffs = td.strOffs[td.callsEndTime.size()-1];
		// add spaces before this call:
		int startOffs = relativeNano(f.startTime_) * cellsPerNanosec;
		int endOffs = relativeNano(f.endTime_) * cellsPerNanosec;
		int spaceCells = std::max(0, startOffs - crtStrOffs);
		crtStr << std::string(spaceCells, ' ');
		crtStrOffs += spaceCells;
		// write this call:
		if (endOffs > startOffs) {
			crtStr << ioModif::RESET << (((ioModif::BG_RGB)colors[f.threadIndex_ % colorsCount]) * (f.deadTime_? 0.5 : 1))
					<< ioModif::BOLD << (f.deadTime_ ? ioModif::FG_GRAY : ioModif::FG_WHITE)
					<< (char)('A' + frameID);
			int callCells = std::max(0, (int)(endOffs - crtStrOffs - 1));
			crtStr << std::string(callCells, ' ') << ioModif::RESET;
			crtStrOffs += callCells + 1;
		}
	}

	// print stats
	for (unsigned i=0; i<threads.size(); i++) {
		auto &t = threads[i];
		os << (ioModif::FG_RGB)colors[i % colorsCount];
		os << ">>>>>>>>>>>>>>>>>>> Thread ["
				<< perf::FrameCapture::getThreadNameForIndex(i)
				<< "] >>>>>>>>>>>>>>>>>\n";
		// print calls:
		for (int i=t.str.size()-1; i>=0; --i)
			os << t.str[i]->str() << "\n";
	}
	// print legend
	os << ioModif::RESET << "\n";
	for (unsigned i=0; i<threads.size(); i++) {
		auto &t = threads[i];
		os << (ioModif::FG_RGB)colors[i % colorsCount];
		std::vector<std::string> legend;
		for (auto &p : t.legend) {
			while (legend.size() <= p.second)
				legend.push_back("");
			legend[p.second] = p.first;
		}
		for (unsigned i=0; i<legend.size(); i++)
			os << ioModif::BOLD << (char)('A' + i) << ioModif::NO_BOLD << " - " << legend[i] << "\n";
	}
	os << ioModif::RESET << "\n\n";
}

#endif // ENABLE_PERF_PROFILING
