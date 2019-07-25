/*
 * Label.cpp
 *
 *  Created on: Apr 15, 2019
 *      Author: bogdan
 */

#include <boglfw/GUI/controls/Label.h>
#include <boglfw/renderOpenGL/Shape2D.h>
#include <boglfw/renderOpenGL/GLText.h>
#include <boglfw/math/math3D.h>
#include <boglfw/GUI/GuiTheme.h>

#include <cstring>

Label::Label(int fontSize, std::string text)
	: text_(text)
	, fontSize_(fontSize)
	, color_(GuiTheme::getButtonTextColor())
{
	setSize(GLText::get()->getTextRect(text, fontSize));
	sizeSetByUser_ = false;
}

Label::~Label() {
}

void Label::draw(RenderContext const& ctx, glm::vec2 frameTranslation, glm::vec2 frameScale) {
	float tx = frameTranslation.x;
	float ty = frameTranslation.y;
	glm::vec2 rc = GLText::get()->getTextRect(text_, fontSize_);

	switch (align_) {
		case LEFT:
			break;
		case CENTER:
			tx += (computedSize().x - rc.x) * 0.5f;
			break;
		case RIGHT:
			tx += computedSize().x - rc.x;
			break;
	}
	switch (baseline_) {
		case TOP:
			ty += rc.y;
			break;
		case MIDDLE:
			ty += (computedSize().y + rc.y) * 0.5f;
			break;
		case BOTTOM:
			ty += computedSize().y;
			break;
	}

	GLText::get()->print(text_, {tx, ty}, fontSize_, color_);
}

void Label::setText(std::string text) {
	text_ = text;
	if (!sizeSetByUser_) {
		setSize(GLText::get()->getTextRect(text_, fontSize_));
		sizeSetByUser_ = false;
	}
}

void Label::setFontSize(int size) {
	fontSize_ = size;
	if (!sizeSetByUser_) {
		setSize(GLText::get()->getTextRect(text_, fontSize_));
		sizeSetByUser_ = false;
	}
}

void Label::setSize(gfcoord size) {
	sizeSetByUser_ = true;
	GuiBasicElement::setSize(size);
}
