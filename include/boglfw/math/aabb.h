/*
 * aabb.h
 *
 *  Created on: Jun 13, 2016
 *      Author: bog
 */

#ifndef MATH_AABB_H_
#define MATH_AABB_H_

#include "math3D.h"
#include <glm/vec3.hpp>

#ifdef WITH_BOX2D
#include "box2glm.h"
#include <Box2D/Collision/b2Collision.h>
#endif // WITH_BOX2D

#include <limits>

struct aabb {
	glm::vec3 vMin;
	glm::vec3 vMax;

	aabb()
		: vMin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max())
		, vMax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()){
		// default creates empty
	}

	aabb(glm::vec3 vMin, glm::vec3 vMax)
		: vMin(vMin), vMax(vMax) {
	}

#ifdef WITH_BOX2D
	aabb(b2AABB const& b2aabb)
		: vMin(b2g(b2aabb.lowerBound), 0.f)
		, vMax(b2g(b2aabb.upperBound), 0.f) {
	}

	operator b2AABB() const {
		b2AABB b;
		b.lowerBound = g2b(vec3xy(vMin));
		b.upperBound = g2b(vec3xy(vMax));
		return b;
	}
#endif // WITH_BOX2D

	aabb(const aabb& x) = default;
	aabb& operator = (aabb const& x) = default;

	bool empty() {
		return vMin.x > vMax.x || vMin.y > vMax.y || vMin.z > vMax.z;
	}

	glm::vec3 center() const {
		return (vMin + vMax) * 0.5f;
	}

	aabb reunion(aabb const& x) const {
		aabb o(*this);
		if (o.vMin.x > x.vMin.x)
			o.vMin.x = x.vMin.x;
		if (o.vMin.y > x.vMin.y)
			o.vMin.y = x.vMin.y;
		if (o.vMin.z > x.vMin.z)
			o.vMin.z = x.vMin.z;
		if (o.vMax.x < x.vMax.x)
			o.vMax.x = x.vMax.x;
		if (o.vMax.y < x.vMax.y)
			o.vMax.y = x.vMax.y;
		if (o.vMax.z < x.vMax.z)
			o.vMax.z = x.vMax.z;
		return o;
	}

	aabb intersect(aabb const& x) const {
		if (x.vMin.x >= vMax.x ||
			x.vMax.x <= vMin.x ||
			x.vMin.y >= vMax.y ||
			x.vMax.y <= vMin.y ||
			x.vMin.z >= vMax.z ||
			x.vMin.z <= vMin.z)
			return aabb();
		return aabb(glm::vec3(max(vMin.x, x.vMin.x), max(vMin.y, x.vMin.y), max(vMin.z, x.vMin.z)),
				glm::vec3(min(vMax.x, x.vMax.x), min(vMax.y, x.vMax.y), min(vMax.z, x.vMax.z)));
	}
	
	aabb offset(glm::vec3 const& o) const {
		return aabb{vMin + o, vMax + o};
	}

	bool intersectSphere(glm::vec3 const& c, float r) const {
		if (c.x + r <= vMin.x ||
			c.y + r <= vMin.y ||
			c.z + r <= vMin.z ||
			c.x - r >= vMax.x ||
			c.y - r >= vMax.y ||
			c.z - r >= vMax.z)
			return false;
		if ((c.x > vMin.x && c.x < vMax.x) ||
			(c.y > vMin.y && c.y < vMax.y) || 
			(c.z > vMin.z && c.z < vMax.z))
			return true;
		float rsq = r*r;
		return
			vec2lenSq(c-vMin) < rsq ||
			vec2lenSq(c-vMax) < rsq ||
			vec2lenSq(c-glm::vec3(vMin.x, vMax.y, vMin.z)) < rsq ||
			vec2lenSq(c-glm::vec3(vMax.x, vMax.y, vMin.z)) < rsq ||
			vec2lenSq(c-glm::vec3(vMax.x, vMin.y, vMin.z)) < rsq || 
			vec2lenSq(c-glm::vec3(vMin.x, vMin.y, vMax.z)) < rsq ||
			vec2lenSq(c-glm::vec3(vMin.x, vMax.y, vMax.z)) < rsq ||
			vec2lenSq(c-glm::vec3(vMax.x, vMin.y, vMax.z)) < rsq;
	}
};

#endif /* MATH_AABB_H_ */
