#pragma once

#include <boglfw/renderOpenGL/ViewportCoord.h>
#include <glm/vec3.hpp>

class RenderContext;

class ScaleDisplay
{
public:
	ScaleDisplay(ViewportCoord pos, int maxPixelsPerUnit);

	void draw(RenderContext const& ctx);

protected:
	ViewportCoord pos_;
	int segmentsXOffset;
	int segmentHeight;
	int labelYOffset;
	int m_MaxSize;
};
