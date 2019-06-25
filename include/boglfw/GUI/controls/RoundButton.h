/*
 * RoundButton.h
 *
 *  Created on: Dec 14, 2018
 *      Author: bog
 */

#ifndef GUI_CONTROLS_ROUND_BUTTON_H_
#define GUI_CONTROLS_ROUND_BUTTON_H_

#include <boglfw/GUI/controls/Button.h>

class RoundButton: public Button {
public:
	RoundButton(float radius, std::string text);
	virtual ~RoundButton() override;

protected:
	virtual void draw(RenderContext const& ctx, glm::vec2 frameTranslation, glm::vec2 frameScale) override;

private:
	glm::vec2 centerPos_ {0.f, 0.f};
	float radius_;
};

#endif /* GUI_CONTROLS_ROUND_BUTTON_H_ */
