#pragma once

#include <boglfw/utils/FlexibleCoordinate.h>
#include <glm/vec3.hpp>

class RenderContext;

class ScaleDisplay
{
public:
	ScaleDisplay(FlexCoordPair pos, int maxPixelsPerUnit);

	void draw(RenderContext const& ctx);

protected:
	FlexCoordPair pos_;
	int segmentsXOffset;
	int segmentHeight;
	int labelYOffset;
	int m_MaxSize;
};
