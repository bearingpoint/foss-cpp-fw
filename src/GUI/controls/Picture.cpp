#include <boglfw/GUI/controls/Picture.h>
#include <boglfw/renderOpenGL/PictureDraw.h>

Picture::Picture(glm::vec2 pos, glm::vec2 size)
	: GuiBasicElement(pos, size) {
}

Picture::~Picture() {
}

void Picture::draw(RenderContext const& ctx, glm::vec2 frameTranslation, glm::vec2 frameScale) {
	if (texture_)
		PictureDraw::get()->draw(texture_, frameTranslation, getSize());
}
