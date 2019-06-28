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
	bool isDescreasing = dir_ == X_RIGHT || dir_ == Y_BOTTOM;
	bool isX = dir_ == X_LEFT || dir_ == X_RIGHT;
	float sz = isX ? ctxSz.x : ctxSz.y;
	float val = value_;
	if (unit_ == PERCENT)
		val *= sz * 0.01f;
	if (isDescreasing)
		val = sz - val;
	return val;

}

float FlexibleCoordinate::get(FlexibleCoordinateContext const& ctx) {
	glm::vec2 ctxSz = ctx.getFlexCoordContextSize();
	return get(ctxSz);
}
