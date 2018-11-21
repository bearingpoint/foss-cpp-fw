/*
 * GuiTheme.h
 *
 *  Created on: Mar 25, 2015
 *      Author: bog
 */

#ifndef GUI_GUITHEME_H_
#define GUI_GUITHEME_H_

#include <glm/vec4.hpp>
#include <memory>

class GuiTheme {
public:
	static void setActiveTheme(std::shared_ptr<GuiTheme> theme) { activeTheme_ = theme; }

	static glm::vec4 getWindowColor() { return activeTheme_->windowColor; }
	static glm::vec4 getWindowFrameColor() { return activeTheme_->windowFrameColor; }
	static glm::vec4 getClientColor() { return activeTheme_->clientColor; }
	static glm::vec4 getClientFrameColor() { return activeTheme_->clientFrameColor; }
	static glm::vec4 getButtonColor() { return activeTheme_->buttonColor; }
	static glm::vec4 getButtonColorHover() { return activeTheme_->buttonColor; }
	static glm::vec4 getButtonColorPressed() { return activeTheme_->buttonColor; }
	static glm::vec4 getButtonFrameColor() { return activeTheme_->buttonFrameColor; }
	static glm::vec4 getButtonTextColor() { return activeTheme_->buttonTextColor; }
	static glm::vec4 getTextFieldColor() { return activeTheme_->textFieldColor; }
	static glm::vec4 getContainerFrameColor() { return activeTheme_->containerFrameColor; }
	static glm::vec4 getContainerBackgroundColor() { return activeTheme_->containerBackgroundColor; }

protected:
	glm::vec4 windowColor;
	glm::vec4 windowFrameColor;
	glm::vec4 clientColor;
	glm::vec4 clientFrameColor;
	glm::vec4 buttonColor;
	glm::vec4 buttonColorHover;
	glm::vec4 buttonColorPressed;
	glm::vec4 buttonFrameColor;
	glm::vec4 buttonTextColor;
	glm::vec4 textFieldColor;
	glm::vec4 containerFrameColor;
	glm::vec4 containerBackgroundColor;

private:
	static std::shared_ptr<GuiTheme> activeTheme_;
};

#endif /* GUI_GUITHEME_H_ */
