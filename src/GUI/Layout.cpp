#include <boglfw/GUI/Layout.h>
#include <boglfw/GUI/GuiContainerElement.h>

// sets the element's position
void Layout::setElementPosition(std::shared_ptr<GuiBasicElement> el, glm::vec2 pos) {
	el->setComputedPosition(pos);
}

// sets the element's size, resolving it according to the element's min/max size rules;
// returns the actual size of the element after the operation
glm::vec2 Layout::setElementSize(std::shared_ptr<GuiBasicElement> el, glm::vec2 size, glm::vec2 const& layoutClientSize) {
	glm::vec2 minSize = el->minSize_.get(layoutClientSize);
	glm::vec2 maxSize = el->maxSize_.get(layoutClientSize);
	if (minSize.x != 0 && size.x < minSize.x)
		size.x = minSize.x;
	if (minSize.y != 0 && size.y < minSize.y)
		size.y = minSize.y;
	if (maxSize.x != 0 && size.x > maxSize.x)
		size.x = maxSize.x;
	if (maxSize.y != 0 && size.y > maxSize.y)
		size.y = maxSize.y;

	el->setComputedSize(size);
	return size;
}

// returns the user-set position of the element
gfcoord Layout::getElementUserPos(std::shared_ptr<GuiBasicElement> el) {
	return el->userPosition_;
}

// returns the user-set size of the element
gfcoord Layout::getElementUserSize(std::shared_ptr<GuiBasicElement> el) {
	return el->userSize_;
}

void Layout::refresh() {
	if (pOwner_)
		pOwner_->refreshLayout();
}
