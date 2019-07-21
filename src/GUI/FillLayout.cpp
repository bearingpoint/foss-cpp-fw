#include <boglfw/GUI/FillLayout.h>

void FillLayout::update(elementIterator first, elementIterator end, glm::vec2 clientSize) {
	for (auto el=first; el!=end; ++el) {
		setElementPosition(*el, {0.f, 0.f});
		setElementSize(*el, clientSize, clientSize);
	}
}
