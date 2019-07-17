#pragma once

#include <boglfw/GUI/Layout.h>

class FillLayout : public Layout {
public:

	FillLayout() {}
	~FillLayout() {}

	virtual void update(std::vector<std::shared_ptr<GuiBasicElement>> &elements, glm::vec2 clientSize) override;

private:
};
