#pragma once

#include <boglfw/GUI/Layout.h>

class GridLayout : public Layout {
public:

	GridLayout() {}
	~GridLayout() {}

	virtual void update(elementIterator first, elementIterator end, glm::vec2 clientSize) override;

private:
};
