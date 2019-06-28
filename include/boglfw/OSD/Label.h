#pragma once

#include <boglfw/utils/FlexibleCoordinate.h>

#include <glm/vec3.hpp>

#include <string>

class RenderContext;

class Label {
public:
	Label(std::string value, FlexCoordPair pos, float textSize, glm::vec3 color);

	void setText(std::string text) { value_ = text; }
	void setColor(glm::vec3 rgb) { color_ = rgb; }
	void setTextSize(float size) { textSize_ = size; }
	void setPos(FlexCoordPair pos) { pos_ = pos; }

	glm::vec2 boxSize() const;

	void draw(RenderContext const& ctx);

	bool drawFrame = true;

protected:
	FlexCoordPair pos_;
	glm::vec3 color_;
	float textSize_;
	std::string value_;
};
