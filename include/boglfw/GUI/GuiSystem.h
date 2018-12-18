/*
 * GuiSystem.h
 *
 *  Created on: Mar 24, 2015
 *      Author: bog
 */

#ifndef GUI_GUISYSTEM_H_
#define GUI_GUISYSTEM_H_

#include <boglfw/GUI/ICaptureManager.h>
#include <memory>
#include <list>

class IGuiElement;
class Viewport;
class InputEvent;

class GuiSystem : public ICaptureManager {
public:
	GuiSystem() = default;
	virtual ~GuiSystem() = default;

	void setMouseCapture(IGuiElement* elementOrNull) override;

	void addElement(std::shared_ptr<IGuiElement> e);
	void addElement(...) = delete;
	void removeElement(std::shared_ptr<IGuiElement> e);
	void draw(Viewport* vp);
	void handleInput(InputEvent &ev);

private:
	std::list<std::shared_ptr<IGuiElement>> elements_;
	std::weak_ptr<IGuiElement> pFocusedElement_;
	std::weak_ptr<IGuiElement> pCaptured_;
	std::weak_ptr<IGuiElement> lastUnderMouse_;

	std::shared_ptr<IGuiElement> getElementUnderMouse(float x, float y);
};

#endif /* GUI_GUISYSTEM_H_ */
