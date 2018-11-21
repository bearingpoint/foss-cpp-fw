/*
 * GuiSystem.cpp
 *
 *  Created on: Mar 24, 2015
 *      Author: bog
 */

#include <boglfw/GUI/GuiSystem.h>
#include <boglfw/GUI/IGuiElement.h>
#include <boglfw/GUI/GuiHelper.h>
#include <boglfw/input/InputEvent.h>
#include <boglfw/utils/log.h>
#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/Renderer.h>
#include <boglfw/math/math3D.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <algorithm>

void GuiSystem::addElement(std::shared_ptr<IGuiElement> e) {
	elements_.push_back(e);
	e->setCaptureManager(this);
}

void GuiSystem::removeElement(std::shared_ptr<IGuiElement> e) {
	elements_.erase(std::find(elements_.begin(), elements_.end(), e));
	e->setCaptureManager(nullptr);
}

void GuiSystem::draw(Viewport* vp) {
	for (auto &e : elements_)
	{
		if (e->isVisible()) {
			vp->renderer()->startBatch();
			glm::vec2 bboxMin, bboxMax;
			e->getBoundingBox(bboxMin, bboxMax);
			e->draw(vp, bboxMin, glm::vec2(1));
		}
	}
}

void GuiSystem::handleInput(InputEvent &ev) {
	if (ev.isConsumed())
		return;
	switch (ev.type) {
	case InputEvent::EV_KEY_DOWN:
		if (pFocusedElement_ && pFocusedElement_->isVisible()) {
			if (pFocusedElement_->keyDown(ev.key))
				ev.consume();
		}
		break;
	case InputEvent::EV_KEY_UP:
		if (pFocusedElement_) {
			if (pFocusedElement_->keyUp(ev.key))
				ev.consume();
		}
		break;
	case InputEvent::EV_KEY_CHAR:
		if (pFocusedElement_) {
			if (pFocusedElement_->keyChar(ev.ch))
				ev.consume();
		}
		break;
	case InputEvent::EV_MOUSE_DOWN:
		if (pCaptured) {
			pCaptured->mouseDown((MouseButtons)ev.mouseButton);
			ev.consume();
		} else {
			if (lastUnderMouse) {
				lastUnderMouse->mouseDown((MouseButtons)ev.mouseButton);
				if (pFocusedElement_ != lastUnderMouse) {
					if (pFocusedElement_) {
						pFocusedElement_->focusLost();
					}
					pFocusedElement_ = lastUnderMouse;
					pFocusedElement_->focusGot();
					// move the clicked element to the top:
					auto it = std::find_if(elements_.begin(), elements_.end(), [&](auto sp) {
						return sp.get() == pFocusedElement_;
					});
					elements_.splice(elements_.end(), elements_, it);
				}
				ev.consume();
			}
		}
		break;
	case InputEvent::EV_MOUSE_UP:
		if (pCaptured) {
			pCaptured->mouseUp((MouseButtons)ev.mouseButton);
			ev.consume();
		} else {
			if (lastUnderMouse) {
				lastUnderMouse->mouseUp((MouseButtons)ev.mouseButton);
				ev.consume();
			}
		}
		break;
	case InputEvent::EV_MOUSE_MOVED:
		if (pCaptured) {
			pCaptured->mouseMoved(glm::vec2{ev.dx, ev.dy}, GuiHelper::parentToLocal(pCaptured, glm::vec2{ev.x, ev.y}));
			ev.consume();
		} else {
			IGuiElement *crt = getElementUnderMouse(ev.x, ev.y);
			if (crt != lastUnderMouse) {
				if (lastUnderMouse)
					lastUnderMouse->mouseLeave();
				lastUnderMouse = crt;
				if (lastUnderMouse) {
					lastUnderMouse->mouseEnter();
				}
			}
			if (lastUnderMouse)
				lastUnderMouse->mouseMoved(glm::vec2{ev.dx, ev.dy}, GuiHelper::parentToLocal(lastUnderMouse, glm::vec2{ev.x, ev.y}));
		}
		break;
	case InputEvent::EV_MOUSE_SCROLL:
		if (pCaptured) {
			pCaptured->mouseScroll(ev.dz);
			ev.consume();
		} else {
			if (lastUnderMouse) {
				lastUnderMouse->mouseScroll(ev.dz);
				ev.consume();
			}
		}
		break;
	default:
		LOGLN("unknown event type: " << ev.type);
	}
}

IGuiElement* GuiSystem::getElementUnderMouse(float x, float y) {
	return GuiHelper::getTopElementAtPosition(elements_, x, y).get();
}
