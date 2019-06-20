#pragma once

#include <boglfw/GUI/GuiContainerElement.h>

class ScrollingContainer : public GuiContainerElement {
public:
	ScrollingContainer(glm::vec2 position, glm::vec2 size);
	virtual ~ScrollingContainer() override;

protected:
	void mouseScroll(float delta) override;
};
