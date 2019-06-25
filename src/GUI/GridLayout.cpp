#include <boglfw/GUI/GridLayout.h>

void GridLayout::update(std::vector<std::shared_ptr<GuiBasicElement>> &elements, glm::vec2 clientSize) {
	for (auto &el : elements) {
		setElementPosition(el, getElementUserPos(el));
		setElementSize(el, getElementUserSize(el));
	}
}
