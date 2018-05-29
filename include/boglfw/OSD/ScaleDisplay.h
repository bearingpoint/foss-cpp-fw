#pragma once

#include <boglfw/renderOpenGL/ViewportCoord.h>
#include <glm/vec3.hpp>

class Viewport;

class ScaleDisplay
{
public:
	ScaleDisplay(ViewportCoord pos, float z, int maxPixelsPerUnit);

	void draw(Viewport* vp);

protected:
	ViewportCoord pos_;
	float z_;
	int segmentsXOffset;
	int segmentHeight;
	int labelYOffset;
	int m_MaxSize;
};
