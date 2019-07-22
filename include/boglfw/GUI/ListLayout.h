#pragma once

#include <boglfw/GUI/Layout.h>

class ListLayout : public Layout {
public:

	enum Direction {
		HORIZONTAL,
		VERTICAL
	};

	enum Alignment {
		LEFT,
		CENTER,
		RIGHT
	};

	enum VerticalAlignment {
		TOP,
		MIDDLE,
		BOTTOM
	};

	ListLayout() {}
	~ListLayout() {}

	void setDirection(Direction dir);
	// set spacing between consecutive items
	void setItemSpacing(FlexCoord spacing);
	void setAlignment(Alignment a);
	void setVerticalAlignment(VerticalAlignment a);

	virtual void update(elementIterator first, elementIterator end, glm::vec2 clientSize) override;

private:
	Direction direction_ = VERTICAL;
	Alignment alignment_ = LEFT;
	VerticalAlignment vertAlignment_ = TOP;
	FlexCoord spacing_ = 0;
};
