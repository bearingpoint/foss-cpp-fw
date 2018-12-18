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

void GuiSystem::setMouseCapture(IGuiElement* elementOrNull) {
	if (!elementOrNull)
		pCaptured_ = {};
	else {
		auto it = std::find_if(elements_.begin(), elements_.end(), [elementOrNull](auto &e) {
			return e.get() == elementOrNull;
		});
		pCaptured_ = it == elements_.end() ? nullptr : *it;
	}
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
		if (auto pf = pFocusedElement_.lock())
			if (pf->isVisible()) {
				if (pf->keyDown(ev.key))
					ev.consume();
			}
		break;
	case InputEvent::EV_KEY_UP:
		if (auto pf = pFocusedElement_.lock()) {
			if (pf->keyUp(ev.key))
				ev.consume();
		}
		break;
	case InputEvent::EV_KEY_CHAR:
		if (auto pf = pFocusedElement_.lock())
			if (pf->isVisible()) {
				if (pf->keyChar(ev.ch))
					ev.consume();
			}
		break;
	case InputEvent::EV_MOUSE_DOWN:
		if (auto pc = pCaptured_.lock()) {
			pc->mouseDown((MouseButtons)ev.mouseButton);
			ev.consume();
		} else {
			if (auto pl = lastUnderMouse_.lock()) {	// clicked on something
				pl->mouseDown((MouseButtons)ev.mouseButton);
				auto pf = pFocusedElement_.lock();
				if (pf != pl) {
					if (pf)
						pf->focusLost();
					pFocusedElement_ = pl;
					pl->focusGot();
					// move the clicked element to the top:
					auto it = std::find(elements_.begin(), elements_.end(), pl);
					elements_.splice(elements_.end(), elements_, it);
				}
				ev.consume();
			} else {	// clicked on nothing
				lastUnderMouse_ = {};
				if (auto pf = pFocusedElement_.lock()) {
					pf->focusLost();
				} else
					pFocusedElement_ = {};
			}
		}
		break;
	case InputEvent::EV_MOUSE_UP:
		if (auto pc = pCaptured_.lock()) {
			pc->mouseUp((MouseButtons)ev.mouseButton);
			ev.consume();
		} else {
			if (auto pl = lastUnderMouse_.lock()) {
				pl->mouseUp((MouseButtons)ev.mouseButton);
				ev.consume();
			}
		}
		break;
	case InputEvent::EV_MOUSE_MOVED:
		if (auto pc = pCaptured_.lock()) {
			pc->mouseMoved(glm::vec2{ev.dx, ev.dy}, GuiHelper::parentToLocal(pc.get(), glm::vec2{ev.x, ev.y}));
			ev.consume();
		} else {
			auto crt = getElementUnderMouse(ev.x, ev.y);
			auto last = lastUnderMouse_.lock();
			if (crt != last) {
				if (last)
					last->mouseLeave();
				lastUnderMouse_ = crt;
				if (crt) {
					crt->mouseEnter();
				}
			}
			if (crt)
				crt->mouseMoved(glm::vec2{ev.dx, ev.dy}, GuiHelper::parentToLocal(crt.get(), glm::vec2{ev.x, ev.y}));
		}
		break;
	case InputEvent::EV_MOUSE_SCROLL:
		if (auto pc = pCaptured_.lock()) {
			pc->mouseScroll(ev.dz);
			ev.consume();
		} else {
			if (auto pl = lastUnderMouse_.lock()) {
				pl->mouseScroll(ev.dz);
				ev.consume();
			}
		}
		break;
	default:
		LOGLN("unknown event type: " << ev.type);
	}
}

std::shared_ptr<IGuiElement> GuiSystem::getElementUnderMouse(float x, float y) {
	return GuiHelper::getTopElementAtPosition(elements_, x, y);
}
