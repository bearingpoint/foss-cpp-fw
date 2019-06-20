/*
 * GuiContainerElement.h
 *
 *  Created on: Mar 25, 2015
 *      Author: bog
 */

#ifndef GUI_GUICONTAINERELEMENT_H_
#define GUI_GUICONTAINERELEMENT_H_

#include <boglfw/GUI/GuiBasicElement.h>
#include <vector>
#include <memory>

class GuiContainerElement: public GuiBasicElement {
public:
	GuiContainerElement(glm::vec2 position, glm::vec2 size);
	virtual ~GuiContainerElement();

	// hit-test a point IN LOCAL COORDINATES
	virtual bool containsPoint(glm::vec2 const& p) const override;

	void addElement(std::shared_ptr<GuiBasicElement> e);
	void removeElement(std::shared_ptr<GuiBasicElement> e);
	std::shared_ptr<GuiBasicElement> findElement(GuiBasicElement* e) const;
	// removes all UI elements from the GUI system
	void clear();
	void setSize(glm::vec2 size) override;
	//std::shared_ptr<GuiBasicElement> getPointedElement() { return elementUnderMouse_; }

	// transparent background will be invisible and will not consume mouse events - they will fall through to the underlying elements
	// by default the background is not transparent.
	void setTransparentBackground(bool transp) { transparentBackground_ = transp; }

	void setClientArea(glm::vec2 offset, glm::vec2 counterOffset);
	void getClientArea(glm::vec2 &outOffset, glm::vec2 &outSize) const;
	glm::vec2 getClientOffset() const { return clientAreaOffset_; }

	virtual bool isContainer() const override { return true; }
	virtual size_t childrenCount() const { return children_.size(); }
	virtual std::shared_ptr<GuiBasicElement> nthChild(size_t n) const { return children_[n]; }

protected:
	friend class GuiSystem;

#warning "must change coordinate space for draw and mouse events"
#warning "all coordinates must be in parent-space. Add support in shape2D for translation stack to simplify drawing code"
	virtual void draw(RenderContext const& ctx, glm::vec2 frameTranslation, glm::vec2 frameScale) override;
	//virtual void mouseDown(MouseButtons button) override;
	//virtual void mouseUp(MouseButtons button) override;
	//virtual void mouseMoved(glm::vec2 delta, glm::vec2 position) override;
	//virtual void clicked(glm::vec2 clickPosition, MouseButtons button) override;
	//virtual bool keyDown(int keyCode) override;
	//virtual bool keyUp(int keyCode) override;
	//virtual bool keyChar(char c) override;

	void setClipping();
	void resetClipping();

private:
	glm::vec2 clientAreaOffset_{0};	// (positive) offset from top left corner of container to top-left corner of client area
	glm::vec2 clientAreaCounterOffset_{0}; // (positive) offset from bottom-right corner of client area to corner of container
	glm::vec2 clientAreaSize_{0};
	bool transparentBackground_ = false;
	std::vector<std::shared_ptr<GuiBasicElement>> children_;
	//std::shared_ptr<GuiBasicElement> elementUnderMouse_ = nullptr;
	//std::shared_ptr<GuiBasicElement> focusedElement_ = nullptr;

	void updateClientArea();
};

#endif /* GUI_GUICONTAINERELEMENT_H_ */
