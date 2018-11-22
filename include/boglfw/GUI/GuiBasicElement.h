/*
 * GuiBasicElement.h
 *
 *  Created on: Mar 25, 2015
 *      Author: bog
 */

#ifndef GUI_GUIBASICELEMENT_H_
#define GUI_GUIBASICELEMENT_H_

#include <boglfw/GUI/IGuiElement.h>
#include <boglfw/GUI/constants.h>

class GuiBasicElement: public IGuiElement {
public:
	GuiBasicElement(glm::vec2 position, glm::vec2 size);
	virtual ~GuiBasicElement();

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
	void getBoundingBox(glm::vec2 &outMin, glm::vec2 &outMax) const override { outMin = bboxMin_; outMax = bboxMax_; }
	virtual bool containsPoint(glm::vec2 const& p) const override;

	bool isMouseIn() { return isMouseIn_; }
	bool isMousePressed(MouseButtons button) { return isMousePressed_[(int)button]; }
	glm::vec2 getLastMousePosition() { return lastMousePosition_; }

protected:
	friend class GuiContainerElement;
	virtual void mouseEnter() override;
	virtual void mouseLeave() override;
	virtual void mouseDown(MouseButtons button) override;
	virtual void mouseUp(MouseButtons button) override;
	virtual void mouseMoved(glm::vec2 delta, glm::vec2 position) override;
	/**
	 * override this to be informed when a valid click has occurred inside the area of the element
	 */
	virtual void clicked(glm::vec2 clickPosition, MouseButtons button) {}

private:

	static constexpr float MAX_CLICK_TRAVEL = 5.f;

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
