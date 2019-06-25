#pragma once

#include <boglfw/GUI/Layout.h>

class GridLayout : public Layout {
public:

	GridLayout() {}
	~GridLayout() {}

	virtual void update(std::vector<std::shared_ptr<GuiBasicElement>> &elements, glm::vec2 clientSize) override;

private:
};
