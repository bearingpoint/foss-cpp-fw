#pragma once

#include <boglfw/GUI/GuiBasicElement.h>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include <vector>
#include <memory>

class GuiContainerElement;
class Viewport;

class Layout {
public:
	virtual ~Layout() {}

	// requests the layout to recompute all elements sizes and positions.
	// [clientSize] is the computed (in pixels) size of the client area of the layout
	// [viewport] is the viewport to use for resolving gvec2 coordinates
	virtual void update(std::vector<std::shared_ptr<GuiBasicElement>> &elements, glm::vec2 clientSize, const Viewport* viewport) = 0;

protected:
	Layout() {}

	friend class GuiContainerElement;
	void setOwner(GuiContainerElement *pOwner) { pOwner_ = pOwner; }

	// sets the element's position
	static void setElementPosition(std::shared_ptr<GuiBasicElement> el, glm::vec2 pos);
	// sets the element's size, resolving it according to the element's min/max size rules;
	// returns the actual size of the element after the operation
	static glm::vec2 setElementSize(std::shared_ptr<GuiBasicElement> el, glm::vec2 size, const Viewport* vp);
	// returns the user-set position of the element
	static gvec2 getElementUserPos(std::shared_ptr<GuiBasicElement> el);
	// returns the user-set size of the element
	static gvec2 getElementUserSize(std::shared_ptr<GuiBasicElement> el);

	// triggers a layout refresh - call this if some properties of the layout have changed
	void refresh();

private:
	GuiContainerElement *pOwner_ = nullptr;
};
