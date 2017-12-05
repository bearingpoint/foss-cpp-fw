/*
 * perfPrint.h
 *
 *  Created on: Dec 30, 2016
 *      Author: bog
 */

#include <boglfw/perf/section.h>
#include <boglfw/perf/frameCapture.h>

#include <vector>
#include <memory>

void printCallTree(std::vector<std::shared_ptr<perf::sectionData>> t, int level);
void printTopHits(std::vector<perf::sectionData> data);
void printFrameCaptureData(std::vector<perf::FrameCapture::frameData> data);

void printCallFrame(perf::sectionData const& s, bool flatMode=false);
void dumpFrameCaptureData(std::vector<perf::FrameCapture::frameData> data);
void printFrameCaptureStatistics(std::vector<perf::FrameCapture::frameData> data);
