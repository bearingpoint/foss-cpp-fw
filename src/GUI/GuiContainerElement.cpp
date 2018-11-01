/*
 * GuiContainerElement.cpp
 *
 *  Created on: Mar 25, 2015
 *      Author: bog
 */

#include <boglfw/GUI/GuiContainerElement.h>
#include <boglfw/GUI/GuiHelper.h>
#include <glm/vec3.hpp>
#include <algorithm>

GuiContainerElement::GuiContainerElement(glm::vec2 position, glm::vec2 size)
	: GuiBasicElement(position, size)
{
}

GuiContainerElement::~GuiContainerElement() {
	children_.clear();
}

void GuiContainerElement::draw(Viewport* vp, glm::vec2 frameTranslation, glm::vec2 frameScale) {
	// draw all children relative to the client area
	frameTranslation += clientAreaOffset_;
	for (auto &e : children_) {
		e->draw(vp, frameTranslation + e->getPosition(), frameScale);
	}
	// TODO draw frame around focused element:
}

void GuiContainerElement::setSize(glm::vec2 size) {
	glm::vec2 oldSize = getSize();
	GuiBasicElement::setSize(size);
	updateClientArea();
	for (auto e : children_) {
		//TODO: adjust e position and size based on anchors
	}
}

void GuiContainerElement::updateClientArea() {
	clientAreaSize_ = getSize() - clientAreaOffset_ - clientAreaCounterOffset_;
}

void GuiContainerElement::addElement(std::shared_ptr<GuiBasicElement> e) {
	children_.push_back(e);
	e->setCaptureManager(getCaptureManager());
}

void GuiContainerElement::removeElement(std::shared_ptr<GuiBasicElement> e) {
	children_.erase(std::find(children_.begin(), children_.end(), e));
}

void GuiContainerElement::mouseDown(MouseButtons button) {
	GuiBasicElement::mouseDown(button);
	if (elementUnderMouse_) {
		if (elementUnderMouse_ != focusedElement_) {
			if (focusedElement_)
				focusedElement_->focusLost();
			focusedElement_ = elementUnderMouse_;
			focusedElement_->focusGot();
		}
		elementUnderMouse_->mouseDown(button);
	}
}

void GuiContainerElement::mouseUp(MouseButtons button) {
	GuiBasicElement::mouseUp(button);
	if (elementUnderMouse_)
		elementUnderMouse_->mouseUp(button);
}

void GuiContainerElement::mouseMoved(glm::vec2 delta, glm::vec2 position) {
	GuiBasicElement::mouseMoved(delta, position);
	glm::vec2 clientPos = position - clientAreaOffset_;
	std::shared_ptr<GuiBasicElement> crt = GuiHelper::getTopElementAtPosition(children_, clientPos.x, clientPos.y);
	if (crt != elementUnderMouse_) {
		if (elementUnderMouse_)
			elementUnderMouse_->mouseLeave();
		elementUnderMouse_ = crt;
		if (elementUnderMouse_)
			elementUnderMouse_->mouseEnter();
	}
	if (elementUnderMouse_)
		elementUnderMouse_->mouseMoved(delta, clientPos - elementUnderMouse_->getPosition());
}

void GuiContainerElement::clicked(glm::vec2 clickPosition, MouseButtons button) {
	GuiBasicElement::clicked(clickPosition, button);
	if (elementUnderMouse_)
		elementUnderMouse_->clicked(clickPosition - clientAreaOffset_ - elementUnderMouse_->getPosition(), button);
}

bool GuiContainerElement::keyDown(int keyCode) {
	if (focusedElement_)
		return focusedElement_->keyDown(keyCode);
	else
		return false;
}

bool GuiContainerElement::keyUp(int keyCode) {
	if (focusedElement_)
		return focusedElement_->keyUp(keyCode);
	else
		return false;
}

bool GuiContainerElement::keyChar(char c) {
	if (focusedElement_)
		return focusedElement_->keyChar(c);
	else
		return false;
}

void GuiContainerElement::setClientArea(glm::vec2 offset, glm::vec2 counterOffset) {
	clientAreaOffset_ = offset;
	clientAreaCounterOffset_ = counterOffset;
	updateClientArea();
}

void GuiContainerElement::getClientArea(glm::vec2 &outOffset, glm::vec2 &outSize) {
	outOffset = clientAreaOffset_;
	outSize = clientAreaSize_;
}
