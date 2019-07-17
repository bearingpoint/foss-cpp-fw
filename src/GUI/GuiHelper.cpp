/*
 * GuiHelper.cpp
 *
 *  Created on: Mar 25, 2015
 *      Author: bog
 */

#include <boglfw/GUI/GuiHelper.h>
#include <boglfw/GUI/GuiBasicElement.h>
#include <boglfw/GUI/GuiContainerElement.h>

namespace GuiHelper {

std::shared_ptr<GuiBasicElement> getTopElementAtPosition(GuiContainerElement const& container, float x, float y) {
	// this assumes the elements are sorted from lowest z-index to highest (last element is top-most)
	// transform the coordinates into client coordinates:
	glm::vec2 clientOffs = container.getComputedClientOffset();
	x -= clientOffs.x;
	y -= clientOffs.y;
	size_t i=container.childrenCount();
	if (i == 0)
		return {};
	else do {
		--i;
		auto c = container.nthChild(i);
		if (!c->isVisible())
			continue;
		glm::vec2 min, max;
		c->getBoundingBox(min, max);
		if (x >= min.x && y >= min.y && x <= max.x && y <= max.y) {
			float lx = x - min.x;	// transform to local coordinates
			float ly = y - min.y;	// transform to local coordinates
			if (c->containsPoint({lx, ly})) {
				if (c->isContainer()) {
					auto el = getTopElementAtPosition(dynamic_cast<GuiContainerElement&>(*c), lx, ly);
					return el ? el : c;
				} else
					return c;
			}
		}
	} while (i > 0);
	return {};
}

glm::vec2 parentToLocal(GuiBasicElement const& el, glm::vec2 pcoord) {
	glm::vec2 bm, bM;
	el.getBoundingBox(bm, bM);
	return pcoord - bm;
}

glm::vec2 viewportToLocal(GuiBasicElement const& el, glm::vec2 vcoord) {
	if (!el.parent())
		return vcoord - el.computedPosition();	// this is the root element
	else {
		return viewportToLocal(*el.parent(), vcoord) - el.parent()->getComputedClientOffset() - el.computedPosition();
	}
}

} // namespace
