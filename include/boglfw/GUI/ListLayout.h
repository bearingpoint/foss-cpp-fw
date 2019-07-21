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
	void setItemSpacing(int pixels);
	void setAlignment(Alignment a);
	void setVerticalAlignment(VerticalAlignment a);

	virtual void update(elementIterator first, elementIterator end, glm::vec2 clientSize) override;

private:
	Direction direction_ = VERTICAL;
	Alignment alignment_ = LEFT;
	VerticalAlignment vertAlignment_ = TOP;
	int spacing_ = 0;
};
