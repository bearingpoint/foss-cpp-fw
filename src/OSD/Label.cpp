#include <boglfw/OSD/Label.h>
#include <boglfw/renderOpenGL/RenderContext.h>
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

void Label::draw() {
	GLText::get()->setViewportFilter(viewportFilter_);
	GLText::get()->print(value_, pos_, z_, textSize_, color_);
	GLText::get()->resetViewportFilter();
	if (drawFrame) {
		glm::vec2 rectSize = boxSize();
		auto vsz = ViewportCoord(rectSize.x, rectSize.y);
		Shape2D::get()->drawRectangle(
				pos_ + ViewportCoord{5, 5},
				z_,
				vsz + ViewportCoord{5, 5},
				color_);
	}
}
