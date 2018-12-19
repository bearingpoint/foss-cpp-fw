/*
 * GuiBasicElement.h
 *
 *  Created on: Mar 25, 2015
 *      Author: bog
 */

#ifndef GUI_GUIBASICELEMENT_H_
#define GUI_GUIBASICELEMENT_H_

#include <boglfw/GUI/constants.h>

class GuiContainerElement;

class GuiBasicElement {
public:
	GuiBasicElement(glm::vec2 position, glm::vec2 size);
	virtual ~GuiBasicElement();
	
	GuiContainerElement* parent() const { return parent_; }

	void setAnchors(Anchors anch) { anchors_ = anch; }
	glm::vec2 getPosition() { return position_; }
	virtual void setPosition(glm::vec2 position);
	glm::vec2 getSize() { return size_; }
	virtual void setSize(glm::vec2 size);
	//void setZIndex(int z) override { zIndex_ = z; }
	bool isVisible() const { return visible_; }
	void show() { visible_ = true; }
	void hide() { visible_ = false; }

	//int zIndex() const override { return zIndex_; }
	void getBoundingBox(glm::vec2 &outMin, glm::vec2 &outMax) const { outMin = bboxMin_; outMax = bboxMax_; }
	
	// return true if the point IN LOCAL COORDINATES is contained within the element's shape;
	// this allows hit-testing on arbitrary shapes, even with holes in them;
	// it is assumed that the bounding box test has been performed prior to this call, the callee is not required to recheck that.
	virtual bool containsPoint(glm::vec2 const& p) const override;

	bool isMouseIn() const { return isMouseIn_; }
	bool isMousePressed(MouseButtons button) const { return isMousePressed_[(int)button]; }
	glm::vec2 getLastMousePosition() const { return lastMousePosition_; }

protected:
	friend class GuiContainerElement;
	GuiContainerElement* parent_ = nullptr;
	
	void setCaptureManager(ICaptureManager* mgr) { captureManager_ = mgr; }
	ICaptureManager* getCaptureManager() const { return captureManager_; }
	
	virtual bool isContainer() const { return false; }
	
	virtual void draw(Viewport* vp, glm::vec2 frameTranslation, glm::vec2 frameScale) = 0;
	
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

	static constexpr float MAX_CLICK_TRAVEL = 5.f;

	ICaptureManager *captureManager_ = nullptr;
	glm::vec2 position_{0};
	glm::vec2 size_{0};
	glm::vec2 bboxMin_{0};
	glm::vec2 bboxMax_{0};
	//int zIndex_ = 0;
	Anchors anchors_ = Anchors::Top | Anchors::Left;
	bool isMouseIn_ = false;
	bool isMousePressed_[3] {false};
	glm::vec2 lastMousePosition_ {0};
	glm::vec2 mouseTravel_[3] {glm::vec2(0)};
	bool visible_ = true;

	void updateBBox();
};

#endif /* GUI_GUIBASICELEMENT_H_ */
