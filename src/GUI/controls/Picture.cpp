#include <boglfw/GUI/controls/Picture.h>
#include <boglfw/renderOpenGL/PictureDraw.h>
#include <boglfw/renderOpenGL/Shape2D.h>

Picture::Picture(glm::vec2 pos, glm::vec2 size)
	: GuiBasicElement(pos, size) {
}

Picture::~Picture() {
}

void Picture::draw(RenderContext const& ctx, glm::vec2 frameTranslation, glm::vec2 frameScale) {
	Shape2D::get()->drawRectangleFilled(frameTranslation, getSize(), bkColor_);
	if (texture_)
		PictureDraw::get()->draw(texture_,
			frameTranslation + glm::vec2(1, 1),
			getSize() - glm::vec2(2, 2),
			enableGammaCorrection_);
}

void Picture::mouseDown(MouseButtons button) {

}

void Picture::mouseUp(MouseButtons button) {

}

void Picture::mouseMoved(glm::vec2 delta, glm::vec2 position) {


}
