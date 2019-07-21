#pragma once

#include <boglfw/GUI/Layout.h>

class FreeLayout : public Layout {
public:

	FreeLayout() {}
	~FreeLayout() {}

	virtual void update(elementIterator first, elementIterator end, glm::vec2 clientSize) override;

private:
};
