#include <boglfw/GUI/Layout.h>
#include <boglfw/GUI/GuiContainerElement.h>

// sets the element's position
void Layout::setElementPosition(std::shared_ptr<GuiBasicElement> el, glm::vec2 pos) {
	el->position_ = pos;
	el->updateBBox();
}

// sets the element's size, resolving it according to the element's min/max size rules;
// returns the actual size of the element after the operation
glm::vec2 Layout::setElementSize(std::shared_ptr<GuiBasicElement> el, glm::vec2 size) {
	el->size_ = size;
	if (el->minSize_.x != 0 && el->size_.x < el->minSize_.x)
		el->size_.x = el->minSize_.x;
	if (el->minSize_.y != 0 && el->size_.y < el->minSize_.y)
		el->size_.y = el->minSize_.y;
	if (el->maxSize_.x != 0 && el->size_.x > el->maxSize_.x)
		el->size_.x = el->maxSize_.x;
	if (el->maxSize_.y != 0 && el->size_.y > el->maxSize_.y)
		el->size_.y = el->maxSize_.y;
	el->updateBBox();

	return el->size_;
}

// returns the user-set position of the element
glm::vec2 Layout::getElementUserPos(std::shared_ptr<GuiBasicElement> el) {
	return el->userPosition_;
}

// returns the user-set size of the element
glm::vec2 Layout::getElementUserSize(std::shared_ptr<GuiBasicElement> el) {
	return el->userSize_;
}

void Layout::refresh() {
	if (pOwner_)
		pOwner_->refreshLayout();
}
