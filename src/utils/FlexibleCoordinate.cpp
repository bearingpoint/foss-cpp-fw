/*
 * FlexibleCoordinate.cpp
 *
 *  Created on: June 28, 2019
 *      Author: bogdan
 */

#include <boglfw/utils/FlexibleCoordinate.h>

FlexibleCoordinate::FlexibleCoordinate(float value, UNIT unit)
	: value_(value)
	, unit_(unit)
{}

float FlexibleCoordinate::get(DIRECTION dir, glm::vec2 ctxSz) {
	bool isDescreasing = dir == X_RIGHT || dir == Y_BOTTOM;
	bool isX = dir == X_LEFT || dir == X_RIGHT;
	float sz = isX ? ctxSz.x : ctxSz.y;
	float val = value_;
	if (unit_ == PERCENT)
		val *= sz * 0.01f;
	if (isDescreasing)
		val = sz - val;
	return val;

}

float FlexibleCoordinate::get(DIRECTION dir, FlexibleCoordinateContext const& ctx) {
	glm::vec2 ctxSz = ctx.getFlexCoordContextSize();
	return get(dir, ctxSz);
}
