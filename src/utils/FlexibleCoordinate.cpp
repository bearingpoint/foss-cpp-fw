/*
 * FlexibleCoordinate.cpp
 *
 *  Created on: June 28, 2019
 *      Author: bogdan
 */

#include <boglfw/utils/FlexibleCoordinate.h>

FlexibleCoordinate::FlexibleCoordinate(DIRECTION dir, float value, UNIT unit)
	: dir_(dir)
	, value_(value)
	, unit_(unit)
{}

float FlexibleCoordinate::get(glm::vec2 ctxSz) {
	if (unit_ == PIXELS)
		return value_;
	else {
		float sz = dir_ == X ? ctxSz.x : ctxSz.y;
		return value_ * sz * 0.01f;
	}
}

float FlexibleCoordinate::get(FlexibleCoordinateContext const& ctx) {
	glm::vec2 ctxSz = ctx.getFlexCoordContextSize();
	return get(ctxSz);
}
