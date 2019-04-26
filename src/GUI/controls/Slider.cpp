#include <boglfw/GUI/controls/Slider.h>
#include <boglfw/utils/assert.h>
#include <boglfw/math/math3D.h>
#include <boglfw/GUI/GuiTheme.h>
#include <boglfw/renderOpenGL/Shape2D.h>
#include <boglfw/renderOpenGL/GLText.h>

#include <sstream>
#include <iomanip>

static constexpr int labelFontSize = 18;
static constexpr int divisionLabelFontSize = 16;
static constexpr float labelsHeight = ceil(divisionLabelFontSize * 1.3);
static constexpr float sliderHeight = 10 + labelsHeight;

Slider::Slider(glm::vec2 pos, float width)
	: GuiBasicElement(pos, glm::vec2{width, sliderHeight}) {
	updateDivisionLabels();
}

void Slider::setRange(float min, float max, unsigned steps) {
	assertDbg(max > min && "invalid interval");
	rangeMin_ = min;
	rangeMax_ = max;
	if (steps) {
		step_ = (rangeMax_ - rangeMin_) / steps;
		displayDivisions_ = steps + 1;
		displayPrecision_ = clamp((int)(4 - log10(rangeMax_ - rangeMin_)), 0, 7);
	} else
		step_ = 0.f;
	updateDivisionLabels();
}

void Slider::setDisplayStyle(unsigned divisions, unsigned divisionLabelStep, unsigned displayPrecision) {
	displayDivisions_ = max(2u, divisions);
	displayPrecision_ = displayPrecision;
	divisionLabelStep_ = divisionLabelStep;
	updateDivisionLabels();
}

void Slider::updateDivisionLabels() {
	divisionLabels_.clear();
	std::stringstream ss;
	for (unsigned i=0; i<displayDivisions_; i++) {
		if (i == 0 || i == displayDivisions_-1 || (i % divisionLabelStep_) == 0) {
			std::stringstream().swap(ss);
			ss << std::fixed << std::setprecision(displayPrecision_) << rangeMin_ + i * step_;
			divisionLabels_.push_back(ss.str());
		} else {
			divisionLabels_.push_back("");
		}
	}
}

void Slider::draw(RenderContext const& ctx, glm::vec2 frameTranslation, glm::vec2 frameScale) {
	glm::vec2 pos = frameTranslation;
	float lowY = pos.y + getSize().y;
	float highY = pos.y + labelsHeight;
	float lowX = pos.x;
	float highX = pos.x + getSize().x;

	// draw base line
	Shape2D::get()->drawLine({lowX, lowY}, {highX, lowY}, GuiTheme::getContainerFrameColor());
	// draw divisions and division labels
	float divisionDeltaX = (highX - lowX) / (displayDivisions_ - 1);
	for (unsigned i=0; i<displayDivisions_; i++) {
		float x = lowX + i * divisionDeltaX;
		Shape2D::get()->drawLine({x, lowY}, {x, highY}, GuiTheme::getContainerFrameColor());
		assertDbg(i < divisionLabels_.size());
		if (!divisionLabels_[i].empty()) {
			auto labelSize = GLText::get()->getTextRect(divisionLabels_[i], divisionLabelFontSize);
			GLText::get()->print(divisionLabels_[i], {x - labelSize.x * 0.5, highY}, divisionLabelFontSize,
				GuiTheme::getContainerFrameColor());
		}
	}
	// draw the tick marker
	// ...

	// draw the label
	if (!label_.empty()) {
		auto labelSize = GLText::get()->getTextRect(label_, labelFontSize);
		GLText::get()->print(label_, {lowX - labelSize.x - 10, lowY - (getSize().y - labelSize.y)*0.5}, labelFontSize,
			GuiTheme::getContainerFrameColor());
	}
}

void Slider::mouseDown(MouseButtons button) {

}

void Slider::mouseMoved(glm::vec2 delta, glm::vec2 position) {

}
