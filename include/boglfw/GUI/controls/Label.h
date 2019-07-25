/*
 * Label.h
 *
 *  Created on: Apr 15, 2019
 *      Author: bogdan
 */

#ifndef GUI_CONTROLS_LABEL_H_
#define GUI_CONTROLS_LABEL_H_

#include <boglfw/GUI/GuiBasicElement.h>
#include <string>

#include <glm/vec3.hpp>

class Label: public GuiBasicElement {
public:
	Label(int fontSize, std::string text);
	virtual ~Label();

	enum ALIGNMENT {
		LEFT,
		CENTER,
		RIGHT
	};
	enum BASELINE {
		TOP,
		MIDDLE,
		BOTTOM
	};

	std::string text() const { return text_; };
	// sets the label text, this will also recompute the label size to adjust to the text unless the user explicitely set a size
	void setText(std::string text);
	void setColor(glm::vec3 color) { color_ = color; }
	// sets the font size, this will also recompute the label size to adjust to the text unless the user explicitely set a size
	void setFontSize(int size);
	void setAlignment(ALIGNMENT align) { align_ = align; }
	void setBaseline(BASELINE baseline) { baseline_ = baseline; }

	void setSize(gfcoord size) override;

	virtual void draw(RenderContext const& ctx, glm::vec2 frameTranslation, glm::vec2 frameScale) override;

protected:
	std::string text_;
	int fontSize_;
	glm::vec3 color_;
	ALIGNMENT align_ = LEFT;
	BASELINE baseline_ = BOTTOM;
	bool sizeSetByUser_ = false;
};

#endif /* GUI_CONTROLS_LABEL_H_ */
