/*
 * GuiBasicElement.h
 *
 *  Created on: Mar 25, 2015
 *      Author: bog
 */

#ifndef GUI_GUIBASICELEMENT_H_
#define GUI_GUIBASICELEMENT_H_

#include <boglfw/GUI/constants.h>
#include <boglfw/renderOpenGL/ViewportCoord.h>

#include <glm/vec2.hpp>

// GUI Vec2 for coordinates and sizes
using gvec2 = ViewportCoord;

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
	gvec2 position() const { return userPosition_; }
	// sets the position - this will only have effect if the element is hosted in a FreeLayout
	virtual void setPosition(gvec2 position);
	// returns the user-set size - the actual size may differ depending on the layout of the container
	gvec2 size() const { return userSize_; }
	// sets the size - this will only have effect if the element is hosted in a FreeLayout
	virtual void setSize(gvec2 size);
	// sets the minimum size of the element - layouts are required to honour this
	virtual void setMinSize(gvec2 minSize);
	// sets the maximum size of the element - layouts are required to honour this
	virtual void setMaxSize(gvec2 maxSize);

	// gets the computed position (in pixels, relative to the parent's client area) of the element;
	// this is the real position computed by the layout
	glm::vec2 computedPosition() const { return computedPosition_; }
	// gets the computed size (in pixels) of the element;
	// this is the real size computed by the layout
	glm::vec2 computedSize() const { return computedSize_; }

	//void setZIndex(int z) override { zIndex_ = z; }
	bool isVisible() const { return visible_; }
	void show() { visible_ = true; }
	void hide() { visible_ = false; }

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

protected:
	friend class GuiContainerElement;
	friend class GuiSystem;
	GuiContainerElement* parent_ = nullptr;

	void setCaptureManager(ICaptureManager* mgr) { captureManager_ = mgr; }
	ICaptureManager* getCaptureManager() const;

	virtual void draw(RenderContext const& ctx, glm::vec2 frameTranslation, glm::vec2 frameScale) = 0;

	virtual void mouseEnter();
	virtual void mouseLeave();
	virtual void mouseDown(MouseButtons button);
	virtual void mouseUp(MouseButtons button);
	// override this to be informed when a valid click has occurred inside the area of the element
	virtual void clicked(glm::vec2 clickPosition, MouseButtons button) {}
	virtual void mouseMoved(glm::vec2 delta, glm::vec2 position);
	virtual void mouseScroll(float delta) {}
	virtual bool keyDown(int keyCode) {return false;}	// return true if the key was consumed
	virtual bool keyUp(int keyCode) {return false;}		// return true if the key was consumed
	virtual bool keyChar(char c) {return false;}		// return true if the key was consumed

	virtual void focusGot() {}
	virtual void focusLost() {}

private:

	friend class Layout;

	static constexpr float MAX_CLICK_TRAVEL = 5.f;

	mutable ICaptureManager *captureManager_ = nullptr;
	glm::vec2 computedPosition_{0};
	glm::vec2 computedSize_{0};
	glm::vec2 bboxMin_{0};
	glm::vec2 bboxMax_{0};

	gvec2 minSize_{0};
	gvec2 maxSize_{0};
	// user defined position for free-layout
	gvec2 userPosition_{0};
	// user defined size for free-layout
	gvec2 userSize_{50, 50};
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
