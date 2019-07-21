#include <boglfw/GUI/GridLayout.h>

void GridLayout::update(elementIterator first, elementIterator end, glm::vec2 clientSize) {
	for (auto el=first; el!=end; ++el) {
		setElementPosition(*el, getElementUserPos(*el).get(clientSize));
		setElementSize(*el, getElementUserSize(*el).get(clientSize), clientSize);
	}
}
