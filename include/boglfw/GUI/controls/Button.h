/*
 * Button.h
 *
 *  Created on: Mar 28, 2015
 *      Author: bog
 */

#ifndef GUI_CONTROLS_BUTTON_H_
#define GUI_CONTROLS_BUTTON_H_

#include <boglfw/GUI/GuiBasicElement.h>
#include <boglfw/utils/Event.h>
#include <string>

class Button: public GuiBasicElement {
public:
	Button(std::string text);

	Event<void()> onClick;

	using buttonHandler = decltype(Button::onClick)::handler_type;

	void setText(std::string text) { text_ = text; }
	void setIcon(...);

	const std::string& text() const { return text_; }

protected:
	virtual void clicked(glm::vec2 clickPosition, MouseButtons button) override;
	virtual void draw(RenderContext const& ctx, glm::vec2 frameTranslation, glm::vec2 frameScale) override;

private:
	std::string text_;
};

#endif /* GUI_CONTROLS_BUTTON_H_ */
