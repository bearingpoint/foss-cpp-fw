#pragma once

#include <boglfw/GUI/GuiContainerElement.h>

class ScrollingContainer : public GuiContainerElement {
public:
	ScrollingContainer();
	virtual ~ScrollingContainer() override;

protected:
	void mouseScroll(float delta) override;
};
