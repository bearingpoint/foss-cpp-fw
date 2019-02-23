/*
 * SignalViewer.cpp
 *
 *  Created on: Jan 24, 2016
 *      Author: bog
 */

#include <boglfw/OSD/SignalViewer.h>
#include <boglfw/renderOpenGL/Shape2D.h>
#include <boglfw/renderOpenGL/GLText.h>
#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/RenderContext.h>
#include <boglfw/math/math3D.h>
#include <boglfw/utils/log.h>
#include <boglfw/perf/marker.h>

#include <sstream>
#include <iomanip>

SignalDataSource::SignalDataSource(std::function<float()> getValue, int maxSamples, float sampleInterval)
	: getValue_(getValue), capacity_(maxSamples), sampleInterval_(sampleInterval) {
	samples_ = new float[maxSamples];
}

SignalDataSource::~SignalDataSource() {
	delete [] samples_;
}

void SignalDataSource::update(float dt) {
	PERF_MARKER_FUNC;
	timeSinceLastSample_ += dt;
	if (timeSinceLastSample_ < sampleInterval_)
		return;

	timeSinceLastSample_ -= sampleInterval_;

	if (n_ < capacity_)
		samples_[n_++] = getValue_();
	else {
		samples_[zero_] = getValue_();
		zero_ = (zero_+1) % n_;
	}
}

SignalViewer::SignalViewer(ViewportCoord pos, ViewportCoord size)
	: pos_(pos), size_(size) {
}

SignalViewer::~SignalViewer() {
}

void SignalViewer::addSignal(std::string const& name, float* pValue, glm::vec3 const& rgb, float sampleInterval, int maxSamples, float minUpperY, float maxLowerY, int displayPrecision) {
	addSignal(name, [pValue] { return *pValue; }, rgb, sampleInterval, maxSamples, minUpperY, maxLowerY, displayPrecision);
}

void SignalViewer::addSignal(std::string const& name, std::function<float()> getValue, glm::vec3 const& rgb, float sampleInterval, int maxSamples, float minUpperY, float maxLowerY, int displayPrecision) {
	sourceInfo_.push_back(DataInfo(std::unique_ptr<SignalDataSource>(new SignalDataSource(getValue, maxSamples, sampleInterval)), name, rgb, minUpperY, maxLowerY, displayPrecision));
}

void SignalViewer::update(float dt) {
	for (auto &s : sourceInfo_)
		s.source_->update(dt);
}

void SignalViewer::draw(RenderContext const& ctx) {
	PERF_MARKER_FUNC;

	constexpr int maxYDivisions = 5;
	constexpr float textSize = 16;
	constexpr float spacePerChar = textSize/2; // pixels
	const glm::vec4 frameColor(1.f, 1.f, 1.f, 0.3f);
	const glm::vec4 divisionColor(1.f, 1.f, 1.f, 0.2f);
	const glm::vec4 divisionLabelColor(1.f, 1.f, 1.f, 0.6f);

	glm::vec2 pos = pos_.xy(ctx.viewport);
	glm::vec2 size = size_.xy(ctx.viewport);

	for (auto &s : sourceInfo_) {
		Shape2D::get()->drawRectangle(pos, size, frameColor);
		float sMin = 1.e20f, sMax = -1.e20f;
		// scan all samples and seek min/max values:
		for (unsigned i=0; i<s.source_->getNumSamples(); i++) {
			float si = s.source_->getSample(i);
			if (si < sMin)
				sMin = si;
			if (si > sMax)
				sMax = si;
		}
		bool noData = false;
		if (!s.source_->getNumSamples()) {
			sMin = sMax = 0;
			noData = true;
		} else {
			// smooth out zoom level:
			if (sMin > s.lastMinValue_)
				sMin = sMin*0.1f + s.lastMinValue_*0.9f;
			if (sMax < s.lastMaxValue_)
				sMax = sMax*0.1f + s.lastMaxValue_*0.9f;
			s.lastMinValue_ = sMin;
			s.lastMaxValue_ = sMax;
		}
		if (sMin > s.maxLowerY_)
			sMin = s.maxLowerY_;
		if (sMax < s.minUpperY_)
			sMax = s.minUpperY_;
		// draw samples:
		float widthPerSample = size.x / s.source_->getCapacity();
		float pixelsPerYUnit = 0;
		if (sMin != sMax)
			pixelsPerYUnit = size.y / (sMax - sMin);
		glm::vec2 prevVertex;
		for (unsigned i=0; i<s.source_->getNumSamples(); i++) {
			glm::vec2 crtVertex { 0, pos.y + size.y - pixelsPerYUnit * (s.source_->getSample(i)-sMin) };
			if (i==0) {
				prevVertex = crtVertex + glm::vec2(pos.x, 0);
				continue;
			} else {
				crtVertex.x += prevVertex.x + widthPerSample;
			}
			Shape2D::get()->drawLine(prevVertex, crtVertex, s.color_);
#warning "build an array and use drawLineStrip"
			prevVertex = crtVertex;
		}
		// draw value axis division lines & labels
		if (sMin * sMax < 0) {
			// zero line is visible
			glm::vec2 zeroY = {0, pos.y + size.y + pixelsPerYUnit*sMin};
			Shape2D::get()->drawLine(pos + zeroY, pos + glm::vec2(size.x, 0) + zeroY, frameColor);
		}
		int nYDivs = maxYDivisions; //min(maxYDivisions, (int)(size.y / yDivisionSize));
		int nDecimals = noData ? 0 : clamp((int)(4 - log10(sMax - sMin)), 0, 7);
		if (s.displayPrecision_ >= 0)
			nDecimals = s.displayPrecision_;
		float yDivisionSize = size.y / nYDivs;
		for (int i=1; i<nYDivs; i++) {
			float lineY = size.y + yDivisionSize * (-i);
			Shape2D::get()->drawLine(pos + glm::vec2{0, lineY}, pos + glm::vec2{size.x, lineY}, divisionColor);
			std::stringstream ss;
			ss << std::fixed << std::setprecision(nDecimals) << sMin + (sMax-sMin) * i / nYDivs;
			GLText::get()->print(ss.str(), pos + glm::vec2{(ss.str().size()+1) * (-spacePerChar), textSize/2 + lineY}, textSize, divisionLabelColor);
		}
		// draw title and current value:
		std::stringstream stitle;
		stitle << s.name_;
		if (!noData)
			stitle << " : " << std::fixed << std::setprecision(nDecimals) << s.source_->getSample(s.source_->getNumSamples()-1);
		else
			stitle << " (no values)";
		GLText::get()->print(stitle.str(), pos, textSize, s.color_);

		pos.y += size.y + 10 + textSize;
	}
}
