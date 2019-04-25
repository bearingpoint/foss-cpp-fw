#pragma once

#include <boglfw/GUI/GuiBasicElement.h>
#include <boglfw/utils/Event.h>

#include <string>
#include <vector>

class Slider : public GuiBasicElement {
public:
	Slider(glm::vec2 pos, glm::vec2 size);

	void setLabel(std::string label) { label_ = label; }
	// sets the value range of the slider.
	// [min] specifies the value for the left-most end.
	// [max] specifies the value for the right-most end.
	// the values are distributed equally along the length of the slider
	// [steps] if greater than zero, then the range will be split into this number of segments,
	//		so there will be [steps]+1 possible positions in total;
	// 		the slider will jump from one position to the next.
	//		if it's zero, then the slider can be freely moved to any intermediate position.
	void setRange(float min, float max, unsigned steps = 0);
	// sets the number of divisions to be drawn;
	// [divisionLabelStep] dictates which divisions get labels;
	//		labels are always drawn for the endpoints;
	//		other labels are drawn every [divisionLabelStep] divisions starting from the left endpoint
	// [displayPrecision] dictates how many decimals are used for labels.
	void setDisplayStyle(unsigned divisions, unsigned divisionLabelStep, unsigned displayPrecision);

	Event<void(float value)> onValueChanged;

protected:
	virtual void draw(RenderContext const& ctx, glm::vec2 frameTranslation, glm::vec2 frameScale) override;
	virtual void mouseDown(MouseButtons button) override;
	virtual void mouseMoved(glm::vec2 delta, glm::vec2 position) override;

	void updateDivisionLabels();

	std::string label_ = "Slider";
	unsigned displayDivisions_ = 11;
	unsigned divisionLabelStep_ = 1;
	unsigned displayPrecision_ = 0;
	float rangeMin_ = 0.f;
	float rangeMax_ = 10.f;
	float step_ = 1.f;
	float value_ = 0.f;

	std::vector<std::string> divisionLabels_;
};
