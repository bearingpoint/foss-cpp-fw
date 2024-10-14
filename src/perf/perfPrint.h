/*
 * perfPrint.h
 *
 *  Created on: Dec 30, 2016
 *	  Author: bog
 */

#ifdef ENABLE_PERF_PROFILING

#include "section.h"
#include "frameCapture.h"

#include <vector>
#include <memory>
#include <iostream>

void printCallTree(std::vector<std::shared_ptr<perf::sectionData>> t, int level, std::ostream &os = std::cout);
void printTopHits(std::vector<perf::sectionData> data, std::ostream &os = std::cout);
void printFrameCaptureData(std::vector<perf::FrameCapture::frameData> data, std::ostream &os = std::cout);

void printCallFrame(perf::sectionData const& s, bool flatMode=false, std::ostream &os = std::cout);
void dumpFrameCaptureData(std::vector<perf::FrameCapture::frameData> data, std::ostream &os = std::cout);
void printFrameCaptureStatistics(std::vector<perf::FrameCapture::frameData> data, std::ostream &os = std::cout);

#endif // ENABLE_PERF_PROFILING
