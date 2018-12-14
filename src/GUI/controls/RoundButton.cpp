/*
 * Button.cpp
 *
 *  Created on: Mar 28, 2015
 *      Author: bog
 */

#include <boglfw/GUI/controls/RoundButton.h>
#include <boglfw/GUI/GuiTheme.h>
#include <boglfw/renderOpenGL/Shape2D.h>
#include <boglfw/renderOpenGL/GLText.h>
#include <boglfw/renderOpenGL/ViewportCoord.h>
#include <boglfw/math/math3D.h>

RoundButton::RoundButton(glm::vec2 centerPos, float radius, std::string text)
	: Button(centerPos - glm::vec2{radius, radius}, glm::vec2{2*radius, 2*radius}, text)
	, centerPos_(centerPos)
	, radius_(radius)
{
}

RoundButton::~RoundButton() {
}

void RoundButton::draw(Viewport* vp, glm::vec2 frameTranslation, glm::vec2 frameScale) {
	glm::vec4 fillColor = isMouseIn() ? isMousePressed(MouseButtons::Left) ? GuiTheme::getButtonColorPressed() : GuiTheme::getButtonColorHover() : GuiTheme::getButtonColor();
	Shape2D::get()->drawCircleFilled(
			vec3xy(frameTranslation) + glm::vec2{radius_, radius_ },	// TODO switch all GUI controls from absolute coordinates to ViewportCoords
			radius_,
			0.f,	// z
			16,		// nSides
			fillColor);
	Shape2D::get()->drawCircle(
			vec3xy(frameTranslation) + glm::vec2{radius_, radius_ },	// TODO switch all GUI controls from absolute coordinates to ViewportCoords
			radius_,
			0.f,	// z
			16,		// nSides
			GuiTheme::getButtonFrameColor());
	auto textRc = GLText::get()->getTextRect(text(), 16);
	glm::vec2 offs = {isMousePressed(MouseButtons::Left) && isMouseIn() ? 1 : 0, isMousePressed(MouseButtons::Left) && isMouseIn() ? 3 : 2};
	glm::vec2 textPos = {frameTranslation.x + radius_ - textRc.x / 2 + offs.x, frameTranslation.y + radius_ + textRc.y / 2 + offs.y};
	float tz = 0.01f;
	GLText::get()->print(text(), textPos, tz, 16, GuiTheme::getButtonTextColor());
}
