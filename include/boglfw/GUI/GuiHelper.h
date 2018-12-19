/*
 * GuiHelper.h
 *
 *  Created on: Mar 25, 2015
 *      Author: bog
 */

#ifndef GUI_GUIHELPER_H_
#define GUI_GUIHELPER_H_

#include <glm/vec2.h>

#include <memory>

class GuiBasicElement;
class GuiContainerElement;

namespace GuiHelper {

// x and y are in local container's space (not client space)
std::shared_ptr<GuiBasicElement> getTopElementAtPosition(GuiContainerElement const& container, float x, float y);
glm::vec2 parentToLocal(GuiBasicElement* el, glm::vec2 pcoord);

} // namespace

#endif /* GUI_GUIHELPER_H_ */
