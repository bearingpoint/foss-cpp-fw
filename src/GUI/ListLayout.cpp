#include <boglfw/GUI/ListLayout.h>

void ListLayout::setItemSpacing(int pixels) {
	spacing_ = pixels;
	refresh();
}

void ListLayout::setAlignment(Alignment a) {
	alignment_ = a;
	refresh();
}

void ListLayout::setVerticalAlignment(VerticalAlignment a) {
	vertAlignment_ = a;
	refresh();
}

void ListLayout::update(std::vector<std::shared_ptr<GuiBasicElement>> &elements, glm::vec2 clientSize) {
	float crtY = 0;
	for (auto &el : elements) {
		glm::vec2 elUserPos = getElementUserPos(el).get(clientSize);
		glm::vec2 elUserSize = getElementUserSize(el).get(clientSize);
		glm::vec2 elSize = setElementSize(el, {clientSize.x, elUserSize.y}, clientSize);
		float x = 0;
		if (alignment_ == RIGHT || alignment_ == CENTER) {
			x = clientSize.x - elSize.x;
			if (alignment_ == CENTER)
				x *= 0.5f;
		}
		setElementPosition(el, {x, crtY});
		crtY += elSize.y + spacing_;
	}
	// take care of vertical alignment:
	float vertOffs = 0.f;
	if (vertAlignment_ != TOP) {
		vertOffs = clientSize.y - (crtY - spacing_);
		if (vertAlignment_ == MIDDLE)
			vertOffs *= 0.5f;
		for (auto &el : elements) {
			auto elPos = el->computedPosition();
			setElementPosition(el, {elPos.x, elPos.y + vertOffs});
		}
	}
}
