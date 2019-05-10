/*
 * GuiSystem.h
 *
 *  Created on: Mar 24, 2015
 *      Author: bog
 */

#ifndef GUI_GUISYSTEM_H_
#define GUI_GUISYSTEM_H_

#include <boglfw/GUI/ICaptureManager.h>
#include <boglfw/GUI/GuiContainerElement.h>
#include <boglfw/utils/Event.h>
#include <memory>

class GuiBasicElement;
class Viewport;
class RenderContext;
class InputEvent;

class GuiSystem : public ICaptureManager {
public:
	// initialize the GUI System on a specified area of the viewport
	GuiSystem(const Viewport* viewport, glm::vec2 position, glm::vec2 size);
	virtual ~GuiSystem() = default;

	// sets a GUI element as capture target - all mouse events will be sent to it regardless of the mouse position;
	// pass nullptr to disable capturing.
	void setMouseCapture(GuiBasicElement* elementOrNull) override;
	// triggers the mouse show/hide event and relies on the owner of this class to handle it.
	// this method has no effect if no show/hide mouse pointer event handler has been set by the GuiSystem owner prior to the call.
	void showMousePointer(bool show) { onMousePointerDisplayRequest.trigger(show); }

	void addElement(std::shared_ptr<GuiBasicElement> e);
	void addElement(...) = delete;
	void removeElement(std::shared_ptr<GuiBasicElement> e);
	// removes all UI elements from the GUI system
	void clear();
	void draw(RenderContext const& ctx);
	void handleInput(InputEvent &ev);

	glm::vec2 getViewportSize() const;

	// subscribe to this event to handle mouse pointer show/hide requests;
	// the bool parameter is true when pointer show is requested and false when hide is requested.
	Event<void(bool)> onMousePointerDisplayRequest;

private:
	const Viewport* viewport_;
	GuiContainerElement rootElement_;
	std::weak_ptr<GuiBasicElement> pFocusedElement_;
	std::weak_ptr<GuiBasicElement> pCaptured_;
	std::weak_ptr<GuiBasicElement> lastUnderMouse_;

	glm::vec2 screenToViewport(glm::vec2 sp) const;
};

#endif /* GUI_GUISYSTEM_H_ */
