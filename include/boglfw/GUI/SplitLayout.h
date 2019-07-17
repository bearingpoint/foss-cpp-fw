#pragma once

#include <boglfw/GUI/Layout.h>

class SplitLayout : public Layout {
public:

	SplitLayout() {}
	~SplitLayout() {}

	virtual void update(std::vector<std::shared_ptr<GuiBasicElement>> &elements, glm::vec2 clientSize) override;

private:
};
