#include <boglfw/OSD/Label.h>
#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/ViewportCoord.h>
#include <boglfw/renderOpenGL/GLText.h>
#include <boglfw/renderOpenGL/Shape2D.h>
#include <boglfw/renderOpenGL/RenderContext.h>
#include <boglfw/math/math3D.h>

Label::Label(std::string value, ViewportCoord pos, float textSize, glm::vec3 color)
	: pos_(pos)
	, color_(color)
	, textSize_(textSize)
	, value_(value) {
}

glm::vec2 Label::boxSize() const {
	return GLText::get()->getTextRect(value_, textSize_);
}

void Label::draw(RenderContext const& ctx) {
	GLText::get()->print(value_, pos_, textSize_, color_);
	if (drawFrame) {
		glm::vec2 rectSize = boxSize() + glm::vec2(5, 5);
		Shape2D::get()->drawRectangle(
				pos_.xy(ctx.viewport()) + glm::vec2{5, 5},
				rectSize,
				color_);
	}
}
