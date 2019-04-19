/*
 * Picture.h
 *
 *  Created on: Apr 15, 2019
 *      Author: bogdan
 */

#ifndef GUI_CONTROLS_PICTURE_H_
#define GUI_CONTROLS_PICTURE_H_

#include <boglfw/GUI/GuiBasicElement.h>
#include <string>

#include <glm/vec3.hpp>

class Picture: public GuiBasicElement {
public:
	Picture(glm::vec2 pos, glm::vec2 size);
	virtual ~Picture();

	void setBackgroundColor(glm::vec3 color) { bkColor_ = color; }
	// sets an OpenGL texture as the picture to be displayed.
	// [applyGammaCorrection] specifies whether gamma correction should be applied to the picture when rendering;
	// if you are displaying a rendered scene, you should specify this
	// as true unless the entire GUI is gamma-corrected in post processing.
	void setPictureTexture(int texId, bool applyGammaCorrection=false) { texture_ = texId; enableGammaCorrection_ = applyGammaCorrection; }
	void enablePictureTransparency(bool enable) { enableAlphaBlend_ = enable; }

	virtual void draw(RenderContext const& ctx, glm::vec2 frameTranslation, glm::vec2 frameScale) override;

protected:
	glm::vec3 bkColor_ {0.f};
	int texture_ = 0;
	bool enableAlphaBlend_ = false;
	bool enableGammaCorrection_ = false;
};

#endif /* GUI_CONTROLS_PICTURE_H_ */
