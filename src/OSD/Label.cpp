#include <boglfw/OSD/Label.h>
#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/ViewportCoord.h>
#include <boglfw/renderOpenGL/GLText.h>
#include <boglfw/renderOpenGL/Shape2D.h>
#include <boglfw/math/math3D.h>

Label::Label(std::string value, ViewportCoord pos, float z, float textSize, glm::vec3 color, std::string viewportFilter)
	: pos_(pos)
	, z_(z)
	, color_(color)
	, textSize_(textSize)
	, value_(value)
	, viewportFilter_(viewportFilter) {
}

glm::vec2 Label::boxSize() const {
	return GLText::get()->getTextRect(value_, textSize_);
}

void Label::draw(Viewport* vp) {
	if (!viewportFilter_.empty() && viewportFilter_ != vp->name())
		return;

	GLText::get()->print(value_, pos_, z_, textSize_, color_);
	if (drawFrame) {
		glm::vec2 rectSize = boxSize() + glm::vec2(5, 5);
		Shape2D::get()->drawRectangle(
				pos_.xy(vp) + glm::vec2{5, 5},
				z_,
				rectSize,
				color_);
	}
}
