#include <boglfw/GUI/FillLayout.h>

void FillLayout::update(std::vector<std::shared_ptr<GuiBasicElement>> &elements, glm::vec2 clientSize) {
	for (auto &el : elements) {
		setElementPosition(el, getElementUserPos(el).get(clientSize));
		setElementSize(el, getElementUserSize(el).get(clientSize), clientSize);
	}
}
