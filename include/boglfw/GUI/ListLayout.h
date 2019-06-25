#pragma once

#include <boglfw/GUI/Layout.h>

class ListLayout : public Layout {
public:

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

	// set spacing between consecutive items
	void setItemSpacing(int pixels);
	void setAlignment(Alignment a);
	void setVerticalAlignment(VerticalAlignment a);

	virtual void update(std::vector<std::shared_ptr<GuiBasicElement>> &elements, glm::vec2 clientSize) override;

private:
	Alignment alignment_ = LEFT;
	VerticalAlignment vertAlignment_ = TOP;
	int spacing_ = 0;
};
