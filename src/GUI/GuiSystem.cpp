/*
 * GuiSystem.cpp
 *
 *  Created on: Mar 24, 2015
 *      Author: bog
 */

#include <boglfw/GUI/GuiSystem.h>
#include <boglfw/GUI/GuiHelper.h>
#include <boglfw/input/InputEvent.h>
#include <boglfw/utils/log.h>
#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/Renderer.h>
#include <boglfw/math/math3D.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <algorithm>

GuiSystem::GuiSystem(Viewport* viewport, glm::vec2 position, glm::vec2 size)
	: viewport_(viewport)
	, rootElement_(position, size) {
	rootElement_.setTransparentBackground(true);
}

void GuiSystem::addElement(std::shared_ptr<GuiBasicElement> e) {
	rootElement_.addElement(e);
	e->setCaptureManager(this);
}

void GuiSystem::removeElement(std::shared_ptr<GuiBasicElement> e) {
	e->setCaptureManager(nullptr);
	rootElement_.removeElement(e);
}

void GuiSystem::setMouseCapture(GuiBasicElement* elementOrNull) {
	if (!elementOrNull)
		pCaptured_ = {};
	else {
		assert(elementOrNull->parent());
		auto captured = elementOrNull->parent()->findElement(elementOrNull);
		assert(captured);
		pCaptured_ = captured;
	}
}

void GuiSystem::draw(Viewport* vp) {
	if (vp == viewport_)
		rootElement_.draw(vp, {0.f, 0.f}, {1.f, 1.f});
}

glm::vec2 GuiSystem::screenToViewport(glm::vec2 sp) const {
	return sp - viewport_->position();
}

void GuiSystem::handleInput(InputEvent &ev) {
	if (ev.isConsumed())
		return;
	glm::vec2 mousePos = screenToViewport({ev.x, ev.y});
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
					//auto it = std::find(elements_.begin(), elements_.end(), pl);
					//elements_.splice(elements_.end(), elements_, it);
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
			pc->mouseMoved(glm::vec2{ev.dx, ev.dy}, GuiHelper::viewportToLocal(*pc.get(), mousePos));
			ev.consume();
		} else {
			auto crt = GuiHelper::getTopElementAtPosition(rootElement_, mousePos.x, mousePos.y);
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
				crt->mouseMoved(glm::vec2{ev.dx, ev.dy}, GuiHelper::viewportToLocal(*crt.get(), mousePos));
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
