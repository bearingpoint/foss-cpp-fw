#include <boglfw/GUI/ListLayout.h>

void ListLayout::setDirection(Direction dir) {
	direction_ = dir;
	refresh();
}

void ListLayout::setItemSpacing(FlexCoord spacing) {
	spacing_ = spacing;
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

void ListLayout::update(elementIterator first, elementIterator end, glm::vec2 clientSize) {
	float crtMain = 0;
	for (auto el=first; el!=end; ++el) {
		glm::vec2 elUserSize = getElementUserSize(*el).get(clientSize);
		glm::vec2 elSize;
		if (direction_ == VERTICAL)
			elSize = setElementSize(*el, {clientSize.x, elUserSize.y}, clientSize);
		else
			elSize = setElementSize(*el, {elUserSize.x, clientSize.y}, clientSize);
		float secondary = 0;
		if (direction_ == VERTICAL) {
			if (alignment_ == RIGHT || alignment_ == CENTER) {
				secondary = clientSize.x - elSize.x;
				if (alignment_ == CENTER)
					secondary *= 0.5f;
			}
		} else {
			if (vertAlignment_ == BOTTOM || vertAlignment_ == MIDDLE) {
				secondary = clientSize.y - elSize.y;
				if (alignment_ == MIDDLE)
					secondary *= 0.5;
			}
		}
		if (direction_ == VERTICAL)
			setElementPosition(*el, {secondary, crtMain});
		else
			setElementPosition(*el, {crtMain, secondary});

		if (direction_ == VERTICAL)
			crtMain += elSize.y + spacing_.get(FlexCoord::Y_TOP, clientSize);
		else
			crtMain += elSize.x + spacing_.get(FlexCoord::X_LEFT, clientSize);
	}
	// take care of alignment along the flow direction:
	if (direction_ == VERTICAL) {
		float vertOffs = 0.f;
		if (vertAlignment_ != TOP) {
			vertOffs = clientSize.y - (crtMain - spacing_.get(FlexCoord::Y_TOP, clientSize));
			if (vertAlignment_ == MIDDLE)
				vertOffs *= 0.5f;
			for (auto el=first; el!=end; ++el) {
				auto elPos = (*el)->computedPosition();
				setElementPosition(*el, {elPos.x, elPos.y + vertOffs});
			}
		}
	} else {
		float horizOffs = 0.f;
		if (alignment_ != LEFT) {
			horizOffs = clientSize.x - (crtMain - spacing_.get(FlexCoord::X_LEFT, clientSize));
			if (alignment_ == CENTER)
				horizOffs *= 0.5f;
			for (auto el=first; el!=end; ++el) {
				auto elPos = (*el)->computedPosition();
				setElementPosition(*el, {elPos.x + horizOffs, elPos.y});
			}
		}
	}
}
