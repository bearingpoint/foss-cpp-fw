/*
 * Label.cpp
 *
 *  Created on: Apr 15, 2019
 *      Author: bogdan
 */

#include <boglfw/GUI/controls/Label.h>
#include <boglfw/renderOpenGL/Shape2D.h>
#include <boglfw/renderOpenGL/GLText.h>
#include <boglfw/renderOpenGL/ViewportCoord.h>
#include <boglfw/math/math3D.h>
#include <boglfw/GUI/GuiTheme.h>

#include <cstring>

Label::Label(glm::vec2 pos, int fontSize, std::string text)
	: GuiBasicElement(pos, GLText::get()->getTextRect(text, fontSize))
	, text_(text)
	, fontSize_(fontSize)
	, color_(GuiTheme::getButtonTextColor())
{
}

Label::~Label() {
}

void Label::draw(RenderContext const& ctx, glm::vec2 frameTranslation, glm::vec2 frameScale) {
	float tx = frameTranslation.x;
	float ty = frameTranslation.y;
	GLText::get()->print(text_, {tx, ty}, fontSize_, color_);
}
