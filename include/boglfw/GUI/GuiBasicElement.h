/*
 * GuiBasicElement.h
 *
 *  Created on: Mar 25, 2015
 *      Author: bog
 */

#ifndef GUI_GUIBASICELEMENT_H_
#define GUI_GUIBASICELEMENT_H_

#include <boglfw/GUI/constants.h>
#include <boglfw/utils/Event.h>
#include <boglfw/utils/FlexibleCoordinate.h>

#include <glm/vec2.hpp>

// GUI flexible coordinate pair
using gfcoord = FlexCoordPair;

class GuiSystem;
class GuiContainerElement;
class ICaptureManager;
class RenderContext;

class GuiBasicElement {
public:
	GuiBasicElement();
	virtual ~GuiBasicElement();

	GuiContainerElement* parent() const { return parent_; }

	// returns the user-set position - the actual position may differ depending on the layout of the container
	gfcoord position() const { return userPosition_; }
	// sets the position - this will only have effect if the element is hosted in a FreeLayout
	virtual void setPosition(gfcoord position);
	// returns the user-set size - the actual size may differ depending on the layout of the container
	gfcoord size() const { return userSize_; }
	// sets the size - this will only have effect if the element is hosted in a FreeLayout
	virtual void setSize(gfcoord size);
	// sets the minimum size of the element - layouts are required to honour this
	virtual void setMinSize(gfcoord minSize);
	// sets the maximum size of the element - layouts are required to honour this
	virtual void setMaxSize(gfcoord maxSize);

	// gets the computed position (in pixels, relative to the parent's client area) of the element;
	// this is the real position computed by the layout
	glm::vec2 computedPosition() const { return computedPosition_; }
	// gets the computed size (in pixels) of the element;
	// this is the real size computed by the layout
	glm::vec2 computedSize() const { return computedSize_; }

	//void setZIndex(int z) override { zIndex_ = z; }
	bool isVisible() const { return visible_; }
	void show() { visible_ = true; onVisibilityChanged.trigger(true); }
	void hide() { visible_ = false; onVisibilityChanged.trigger(false); }

	//int zIndex() const override { return zIndex_; }

	// returns the computed bounding box, in pixels, in parent's client space
	void getBoundingBox(glm::vec2 &outMin, glm::vec2 &outMax) const { outMin = bboxMin_; outMax = bboxMax_; }

	// return true if the point IN LOCAL COORDINATES is contained within the element's shape;
	// this allows hit-testing on arbitrary shapes, even with holes in them;
	// it is assumed that the bounding box test has been performed prior to this call, the callee is not required to recheck that.
	virtual bool containsPoint(glm::vec2 const& p) const;

	bool isMouseIn() const { return isMouseIn_; }
	bool isMousePressed(MouseButtons button) const { return isMousePressed_[(int)button]; }
	glm::vec2 getLastMousePosition() const { return lastMousePosition_; }

	virtual bool isContainer() const { return false; }

	// triggered when the element has been added/removed to/from a parent
	Event<void(GuiContainerElement* parent)> onParentChanged;
	// triggered when the COMPUTED position of the element has changed
	Event<void(glm::vec2)> onPositionChanged;
	// triggered when the COMPUTED size of the element has changed
	Event<void(glm::vec2)> onSizeChanged;
	Event<void()> onMouseEnter;
	Event<void()> onMouseLeave;
	Event<void(MouseButtons)> onMouseDown;
	Event<void(MouseButtons)> onMouseUp;
	Event<void(glm::vec2 relPos, MouseButtons)> onClicked;
	Event<void(glm::vec2 relPos, glm::vec2 dist)> onMouseMoved;
	Event<void(float)> onMouseScroll;
	Event<void(int)> onKeyDown;
	Event<void(int)> onKeyUp;
	Event<void(char)> onKeyChar;
	Event<void()> onFocusGot;
	Event<void()> onFocusLost;
	Event<void(bool)> onVisibilityChanged;

protected:
	friend class GuiContainerElement;
	friend class GuiSystem;
	GuiContainerElement* parent_ = nullptr;

	void setCaptureManager(ICaptureManager* mgr) { captureManager_ = mgr; }
	ICaptureManager* getCaptureManager() const;

	void setParent(GuiContainerElement* parent) { parent_ = parent; onParentChanged.trigger(parent_); }

	virtual void draw(RenderContext const& ctx, glm::vec2 frameTranslation, glm::vec2 frameScale) = 0;

	virtual void mouseEnter();
	virtual void mouseLeave();
	virtual void mouseDown(MouseButtons button);
	virtual void mouseUp(MouseButtons button);
	// override this to be informed when a valid click has occurred inside the area of the element
	virtual void clicked(glm::vec2 clickPosition, MouseButtons button) { onClicked.trigger(clickPosition, button); }
	virtual void mouseMoved(glm::vec2 delta, glm::vec2 position);
	virtual void mouseScroll(float delta) { onMouseScroll.trigger(delta); }
	virtual bool keyDown(int keyCode) { onKeyDown.trigger(keyCode); return false; }	// return true if the key was consumed
	virtual bool keyUp(int keyCode) { onKeyUp.trigger(keyCode); return false;}		// return true if the key was consumed
	virtual bool keyChar(char c) { onKeyChar.trigger(c); return false;}		// return true if the key was consumed

	virtual void focusGot() { onFocusGot.trigger(); }
	virtual void focusLost() { onFocusLost.trigger(); }

private:

	friend class Layout;

	virtual void setComputedPosition(glm::vec2 pos);
	virtual void setComputedSize(glm::vec2 sz);

	static constexpr float MAX_CLICK_TRAVEL = 5.f;

	mutable ICaptureManager *captureManager_ = nullptr;
	glm::vec2 computedPosition_{0};
	glm::vec2 computedSize_{50, 50};
	glm::vec2 bboxMin_{0};
	glm::vec2 bboxMax_{50, 50};

	gfcoord minSize_{0, 0};
	gfcoord maxSize_{0, 0};
	// user defined position for free-layout
	gfcoord userPosition_{0, 0};
	// user defined size for free-layout
	gfcoord userSize_{50, 50};
	//int zIndex_ = 0;

	//Anchors anchors_ = Anchors::Top | Anchors::Left;
	bool isMouseIn_ = false;
	bool isMousePressed_[3] {false};
	glm::vec2 lastMousePosition_ {0};
	glm::vec2 mouseTravel_[3] {glm::vec2(0)};
	bool visible_ = true;

	void updateBBox();
};

#endif /* GUI_GUIBASICELEMENT_H_ */
