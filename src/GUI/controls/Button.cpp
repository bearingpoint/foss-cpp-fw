/*
 * Button.cpp
 *
 *  Created on: Mar 28, 2015
 *      Author: bog
 */

#include <boglfw/GUI/controls/Button.h>
#include <boglfw/GUI/GuiTheme.h>
#include <boglfw/renderOpenGL/Shape2D.h>
#include <boglfw/renderOpenGL/GLText.h>
#include <boglfw/renderOpenGL/ViewportCoord.h>
#include <boglfw/math/math3D.h>

Button::Button(glm::vec2 pos, glm::vec2 size, std::string text)
	: GuiBasicElement(pos, size)
	, text_(text) {
}

Button::~Button() {
}

void Button::clicked(glm::vec2 clickPosition, MouseButtons button) {
	if (button == MouseButtons::Left)
		onClick.trigger(this);
}

void Button::draw(Viewport* vp, glm::vec2 frameTranslation, glm::vec2 frameScale) {
	glm::vec4 fillColor = isMouseIn() ? (
							isMousePressed(MouseButtons::Left) ?
								GuiTheme::getButtonColorPressed()
							: GuiTheme::getButtonColorHover()
						) : GuiTheme::getButtonColor();
	Shape2D::get()->drawRectangleFilled(
			vec3xy(frameTranslation) + glm::vec2(2,2),	// TODO switch all GUI controls from absolute coordinates to ViewportCoords
			0,
			(getSize()-glm::vec2(4,4)) * frameScale,
			fillColor);
	Shape2D::get()->drawRectangle(
			vec3xy(frameTranslation),
			0,
			getSize() * frameScale,
			GuiTheme::getButtonFrameColor());
	auto textRc = GLText::get()->getTextRect(text_, 16);
	glm::vec2 offs = {isMousePressed(MouseButtons::Left) && isMouseIn() ? 1 : 0, isMousePressed(MouseButtons::Left) && isMouseIn() ? 3 : 2};
	glm::vec2 textPos = {frameTranslation.x + getSize().x / 2 - textRc.x / 2 + offs.x, frameTranslation.y + getSize().y / 2 + textRc.y / 2 + offs.y};
	float tz = 0.01f;
	GLText::get()->print(text_, textPos, tz, 16, GuiTheme::getButtonTextColor());
}
