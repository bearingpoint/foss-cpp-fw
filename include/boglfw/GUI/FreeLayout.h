#pragma once

#include <boglfw/GUI/Layout.h>

class FreeLayout : public Layout {
public:

	FreeLayout() {}
	~FreeLayout() {}

	virtual void update(std::vector<std::shared_ptr<GuiBasicElement>> &elements, glm::vec2 clientSize) override;

private:
};
