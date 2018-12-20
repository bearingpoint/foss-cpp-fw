/*
 * GuiHelper.h
 *
 *  Created on: Mar 25, 2015
 *      Author: bog
 */

#ifndef GUI_GUIHELPER_H_
#define GUI_GUIHELPER_H_

#include <glm/vec2.hpp>

#include <memory>

class GuiBasicElement;
class GuiContainerElement;

namespace GuiHelper {

// searches recursively for the top-most element at the specified point.
// the function assumes the elements in containers are sorted from lowest to highest z-index (top-most last).
// if a container is found at the position, the function recurses within the container to find the deepest element at the point.
// if a container's body is at the point, but none of its children and the container's background is not trasparent, the container is returned.
// x and y are in local container's space (not client space)
std::shared_ptr<GuiBasicElement> getTopElementAtPosition(GuiContainerElement const& container, float x, float y);

// transforms coordinates from parent's client space into element's local space
glm::vec2 parentToLocal(GuiBasicElement const& el, glm::vec2 pcoord);

// transforms coordinates from viewport's space into element's local space
glm::vec2 viewportToLocal(GuiBasicElement const& el, glm::vec2 vcoord);

} // namespace

#endif /* GUI_GUIHELPER_H_ */
