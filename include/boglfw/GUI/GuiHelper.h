/*
 * GuiHelper.h
 *
 *  Created on: Mar 25, 2015
 *      Author: bog
 */

#ifndef GUI_GUIHELPER_H_
#define GUI_GUIHELPER_H_

#include <boglfw/GUI/IGuiElement.h>

#include <glm/vec2.hpp>

#include <memory>
#include <vector>

class IGuiElement;
class GuiBasicElement;

class GuiHelper {
public:
	template <class C, class T=typename C::value_type>
	static T getTopElementAtPosition(C collection, float x, float y) {
		if (!collection.size())
			return {};
		auto it = collection.end();
		do {
			--it;
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

	static glm::vec2 parentToLocal(IGuiElement* el, glm::vec2 pcoord) {
		glm::vec2 bm, bM;
		el->getBoundingBox(bm, bM);
		return pcoord - bm;
	}
};

#endif /* GUI_GUIHELPER_H_ */
