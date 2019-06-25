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
		glm::vec2 elUserSize = getElementUserSize(el);
		glm::vec2 elSize = setElementSize(el, {clientSize.x, elUserSize.y});
		float x = 0;
		if (alignment_ == RIGHT || alignment_ == CENTER) {
			x = clientSize.x - elSize.x;
			if (alignment_ == CENTER)
				x *= 0.5f;
		}
		setElementPosition(el, {x, crtY});
		crtY += elSize.y + spacing_;
	}
}
