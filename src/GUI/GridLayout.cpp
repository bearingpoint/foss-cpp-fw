#include <boglfw/GUI/GridLayout.h>
#include <boglfw/utils/assert.h>

void GridLayout::update(std::vector<std::shared_ptr<GuiBasicElement>> &elements, glm::vec2 clientSize, const Viewport* viewport) {
	assertDbg(viewport);
	for (auto &el : elements) {
		setElementPosition(el, getElementUserPos(el).xy(*viewport));
		setElementSize(el, getElementUserSize(el).xy(*viewport), viewport);
	}
}
