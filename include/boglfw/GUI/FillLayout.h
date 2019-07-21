#pragma once

#include <boglfw/GUI/Layout.h>

class FillLayout : public Layout {
public:

	FillLayout() {}
	~FillLayout() {}

	virtual void update(elementIterator first, elementIterator end, glm::vec2 clientSize) override;

private:
};
