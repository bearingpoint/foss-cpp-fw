#include <boglfw/GUI/controls/Picture.h>
#include <boglfw/renderOpenGL/PictureDraw.h>
#include <boglfw/renderOpenGL/Shape2D.h>

Picture::Picture() {
}

Picture::~Picture() {
}

void Picture::draw(RenderContext const& ctx, glm::vec2 frameTranslation, glm::vec2 frameScale) {
	Shape2D::get()->drawRectangleFilled(frameTranslation, computedSize(), bkColor_);
	if (texture_)
		PictureDraw::get()->draw(texture_,
			frameTranslation + glm::vec2(1, 1),
			computedSize() - glm::vec2(2, 2),
			enableGammaCorrection_);
}

void Picture::mouseDown(MouseButtons button) {
	GuiBasicElement::mouseDown(button);
	if (button == MouseButtons::Left)
		onStartDrag.trigger(getLastMousePosition().x, getLastMousePosition().y);
}

void Picture::mouseUp(MouseButtons button) {
	GuiBasicElement::mouseUp(button);
	if (button == MouseButtons::Left)
		onEndDrag.trigger();
}

void Picture::mouseMoved(glm::vec2 delta, glm::vec2 position) {
	GuiBasicElement::mouseMoved(delta, position);
	if (isMousePressed(MouseButtons::Left))
		onDrag.trigger(delta.x, delta.y);
}

void Picture::mouseScroll(float delta) {
	GuiBasicElement::mouseScroll(delta);
	onScroll.trigger(delta);
}
