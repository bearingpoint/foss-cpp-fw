/*
 * TextField.cpp
 *
 *  Created on: Mar 29, 2015
 *      Author: bogdan
 */

#include <boglfw/GUI/controls/TextField.h>
#include <boglfw/renderOpenGL/Shape2D.h>
#include <boglfw/renderOpenGL/GLText.h>
#include <boglfw/renderOpenGL/ViewportCoord.h>
#include <boglfw/math/math3D.h>
#include <boglfw/GUI/GuiTheme.h>

#include <cstring>

TextField::TextField(glm::vec2 pos, glm::vec2 size, std::string initialText)
	: GuiBasicElement(pos, size)
{
	strncpy(textBuffer_, initialText.c_str(), maxTextbufferSize);
}

TextField::~TextField() {
}

bool TextField::keyDown(int keyCode) {
	return false;
}

void TextField::draw(RenderContext const& ctx, glm::vec2 frameTranslation, glm::vec2 frameScale) {
	Shape2D::get()->drawRectangleFilled(
			vec3xy(frameTranslation)+glm::vec2(2,2),
			(getSize()-glm::vec2(4,4)) * frameScale,
			GuiTheme::getTextFieldColor());
	Shape2D::get()->drawRectangle(
			vec3xy(frameTranslation),
			getSize() * frameScale,
			GuiTheme::getButtonFrameColor());
	float tx = frameTranslation.x + 10;
	float ty = frameTranslation.y + 20;
	GLText::get()->print(textBuffer_, {tx, ty}, 14, GuiTheme::getButtonTextColor());
}

bool TextField::keyChar(char c) {
	if (bufSize_ < maxTextbufferSize-1) {
		for (int i=bufPos_+1; i<=bufSize_; i++)
			textBuffer_[i] = textBuffer_[i-1];
		bufSize_++;
		textBuffer_[bufSize_] = 0;
		textBuffer_[bufPos_++] = c;
	}
	return true;
}
