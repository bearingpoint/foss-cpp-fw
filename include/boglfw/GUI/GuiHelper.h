/*
 * GuiHelper.h
 *
 *  Created on: Mar 25, 2015
 *      Author: bog
 */

#ifndef GUI_GUIHELPER_H_
#define GUI_GUIHELPER_H_

#include <memory>
#include <vector>
#include <glm/vec2.hpp>

class IGuiElement;
class GuiBasicElement;

class GuiHelper {
public:
	template <class C, class T=typename C::value_type>
	static T getTopElementAtPosition(C collection, float x, float y) {
		if (!collection.size())
			return {};
		auto it = --collection.end();
		do {
			glm::vec2 min, max;
			(*it)->getBoundingBox(min, max);
			if (x >= min.x && y >= min.y && x <= max.x && y <= max.y)
				return *it;
			--it;
		} while (it != collection.begin());
		return {};
		/*decltype(collection) vec;
		for (auto &e : collection) {
			glm::vec2 min, max;
			e->getBoundingBox(min, max);
			if (x >= min.x && y >= min.y && x <= max.x && y <= max.y)
				vec.push_back(e);
		}
		if (!vec.size())
			return {};
		T &top = vec.front();
		int topZ = top->zIndex();
		for (auto &e : vec)
			if (e->zIndex() >= topZ) {
				topZ = e->zIndex();
				top = e;
			}
		return top;*/
	}
};

#endif /* GUI_GUIHELPER_H_ */
