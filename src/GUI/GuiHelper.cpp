/*
 * GuiHelper.cpp
 *
 *  Created on: Mar 25, 2015
 *      Author: bog
 */

#include <boglfw/GUI/GuiHelper.h>
#include <boglfw/GUI/GuiBasicElement>
#include <boglfw/GUI/GuiContainerElement.h>

namespace GuiHelper {

std::shared_ptr<GuiBasicElement> getTopElementAtPosition(GuiContainerElement const& container, float x, float y) {
	size_t i=container.childrenCount();
	here left
	do {
		--i;
		if (!(*it)->isVisible())
			continue;
		glm::vec2 min, max;
		(*it)->getBoundingBox(min, max);
		if (x >= min.x && y >= min.y && x <= max.x && y <= max.y) {
			float lx = x - min.x;	// transform to local coordinates
			float ly = y - min.y;	// transform to local coordinates
			if ((*it)->containsPoint({lx, ly}))
				return *it;
		}
	} while (it != collection.begin());
	return {};
}

glm::vec2 parentToLocal(IGuiElement* el, glm::vec2 pcoord) {
	glm::vec2 bm, bM;
	el->getBoundingBox(bm, bM);
	return pcoord - bm;
}

} // namespace
