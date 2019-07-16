/*
 * FlexibleCoordinate.h
 *
 *  Created on: June 28, 2019
 *      Author: bogdan
 */

#pragma once

#include <glm/vec2.hpp>

class FlexibleCoordinateContext {
public:
	// this method returns the size (width and height) of the coordinate context
	virtual glm::vec2 getFlexCoordContextSize() const = 0;
};

class FlexibleCoordinate {
public:
	enum UNIT {
		PIXELS,
		PERCENT
	};
	enum DIRECTION {
		X_LEFT,
		X_RIGHT,
		Y_TOP,
		Y_BOTTOM
	};
	
	FlexibleCoordinate() = default;

	FlexibleCoordinate(float value, UNIT unit=PIXELS);
	FlexibleCoordinate(FlexibleCoordinate const&) = default;
	FlexibleCoordinate(FlexibleCoordinate &&) = default;

	FlexibleCoordinate& operator = (FlexibleCoordinate const&) = default;
	FlexibleCoordinate& operator = (FlexibleCoordinate &&) = default;

	// return the value in pixels depending on direction and context
	float get(DIRECTION dir, FlexibleCoordinateContext const& ctx);

	// return the value in pixels given the context size
	float get(DIRECTION dir, glm::vec2 ctxSize);

private:
	float value_ = 0;
	UNIT unit_ = PIXELS;
};

using FlexCoord = FlexibleCoordinate;

class FlexCoordPair {
public:
	FlexCoord x;
	FlexCoord y;

	enum ANCHOR_X {
		LEFT,
		RIGHT
	};
	enum ANCHOR_Y {
		TOP,
		BOTTOM
	};
	
	FlexCoordPair() = default;

	FlexCoordPair(float x, float y, FlexCoord::UNIT unit = FlexCoord::PIXELS, ANCHOR_X ancX = LEFT, ANCHOR_Y ancY = TOP)
		: x(x, unit)
		, y(y, unit)
		, dirX_(ancX == LEFT ? FlexCoord::X_LEFT : FlexCoord::X_RIGHT)
		, dirY_(ancY == TOP ? FlexCoord::Y_TOP : FlexCoord::Y_BOTTOM)
	{}

	FlexCoordPair(glm::vec2 v) : FlexCoordPair(v.x, v.y) {}

	glm::vec2 get(FlexibleCoordinateContext const& ctx) {
		return glm::vec2{x.get(dirX_, ctx), y.get(dirY_, ctx)};
	}

	glm::vec2 get(glm::vec2 const& ctxSize) {
		return glm::vec2{x.get(dirX_, ctxSize), y.get(dirY_, ctxSize)};
	}

	FlexCoordPair(FlexCoordPair const&) = default;
	FlexCoordPair(FlexCoordPair &&) = default;

	FlexCoordPair& operator = (FlexCoordPair const&) = default;
	FlexCoordPair& operator = (FlexCoordPair &&) = default;
	
private:
	FlexCoord::DIRECTION dirX_;
	FlexCoord::DIRECTION dirY_;
};
