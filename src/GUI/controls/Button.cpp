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
#include <boglfw/math/math3D.h>

Button::Button(std::string text)
	: text_(text) {
}

void Button::clicked(glm::vec2 clickPosition, MouseButtons button) {
	if (button == MouseButtons::Left)
		onClick.trigger();
}

void Button::draw(RenderContext const& ctx, glm::vec2 frameTranslation, glm::vec2 frameScale) {
	glm::vec4 fillColor = isMouseIn() ? (
							isMousePressed(MouseButtons::Left) ?
								GuiTheme::getButtonColorPressed()
							: GuiTheme::getButtonColorHover()
						) : GuiTheme::getButtonColor();
	Shape2D::get()->drawRectangleFilled(
			frameTranslation + glm::vec2(2,2),
			(computedSize()-glm::vec2(4,4)) * frameScale,
			fillColor);
	Shape2D::get()->drawRectangle(
			frameTranslation,
			computedSize() * frameScale,
			GuiTheme::getButtonFrameColor());
	auto textRc = GLText::get()->getTextRect(text_, 18);
	glm::vec2 offs = {isMousePressed(MouseButtons::Left) && isMouseIn() ? 1 : 0, isMousePressed(MouseButtons::Left) && isMouseIn() ? 3 : 2};
	glm::vec2 textPos = {frameTranslation.x + computedSize().x / 2 - textRc.x / 2 + offs.x, frameTranslation.y + computedSize().y / 2 + textRc.y / 2 + offs.y};
	GLText::get()->print(text_, textPos, 18, GuiTheme::getButtonTextColor());
}
