#pragma once

#include <boglfw/GUI/GuiBasicElement.h>
#include <boglfw/utils/Event.h>

#include <string>
#include <vector>

class Slider : public GuiBasicElement {
public:
	Slider(glm::vec2 pos, float width);

	void setLabel(std::string label) { label_ = label; }
	// sets the value range of the slider.
	// [min] specifies the value for the left-most end.
	// [max] specifies the value for the right-most end.
	// the values are distributed equally along the length of the slider
	// [stepSize] if greater than zero, then the slider will be constrained to jump to values
	//		that are equal to [min] + multiple of stepSize
	//		so there will be (([max]-[min])/[stepSize]) + 1 possible positions in total;
	// 		the slider will jump from one position to the next.
	//		if it's zero, then the slider can be freely moved to any intermediate position.
	void setRange(float min, float max, float stepSize = 0);
	// sets the number of divisions to be drawn;
	// [divisionLabelStep] dictates which divisions get labels;
	//		labels are always drawn for the endpoints;
	//		other labels are drawn every [divisionLabelStep] divisions starting from the left endpoint
	// [displayPrecision] dictates how many decimals are used for labels.
	void setDisplayStyle(unsigned divisions, unsigned divisionLabelStep, unsigned displayPrecision);

	// sets the value of the slider; the value provided will be clamped between [rangeMin, rangeMax]
	void setValue(float val);

	Event<void(float value)> onValueChanged;

protected:
	virtual void draw(RenderContext const& ctx, glm::vec2 frameTranslation, glm::vec2 frameScale) override;
	virtual void mouseDown(MouseButtons button) override;
	virtual void mouseUp(MouseButtons button) override;
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
