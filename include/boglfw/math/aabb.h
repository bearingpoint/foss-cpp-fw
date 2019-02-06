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

struct AABB {
	glm::vec3 vMin;
	glm::vec3 vMax;

	AABB()
		: vMin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max())
		, vMax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()){
		// default creates empty
	}

	AABB(glm::vec3 vMin, glm::vec3 vMax)
		: vMin(vMin), vMax(vMax) {
	}

	// returns an empty AABB
	static AABB empty() {
		return AABB();
	}

#ifdef WITH_BOX2D
	AABB(b2AABB const& b2aabb)
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

	AABB(const AABB& x) = default;
	AABB& operator = (AABB const& x) = default;

	bool isEmpty() {
		return vMin.x > vMax.x || vMin.y > vMax.y || vMin.z > vMax.z;
	}

	glm::vec3 center() const {
		return (vMin + vMax) * 0.5f;
	}

	glm::vec3 size() const {
		return { vMax.x - vMin.x, vMax.y - vMin.y, vMax.z - vMin.z };
	}

	// qualifies this AABB against a plane in space.
	// Returns:
	//	-1 if the entire AABB is on the negative side of the plane (even if at least one vertex is exactly on the plane),
	//	+1 if the entire AABB is on the positive side of the plane (even if at least one vertex is exactly on the plane),
	//	 0 if the AABB spans both sides of the plane (some vertices are strictly on the positive side, and some strictly on the negative).
	int qualifyPlane(glm::vec4 const& plane) const {
		glm::vec3 verts[8] {
			vMin,
			{vMin.x, vMin.y, vMax.z},
			{vMin.x, vMax.y, vMax.z},
			{vMin.x, vMax.y, vMin.z},
			vMax,
			{vMax.x, vMin.y, vMin.z},
			{vMax.x, vMin.y, vMax.z},
			{vMax.x, vMax.y, vMin.z},
		};
		int nNeg = 0, nPos = 0;
		for (int i=0; i<8; i++) {
			float q = verts[i].x*plane.x + verts[i].y*plane.y + verts[i].z*plane.z + plane.w;
			if (q > 0)
				nPos++;
			else if (q < 0)
				nNeg++;
		}
		return sign(nPos - nNeg);
	}

	// expand this aabb to include the new point
	void expand(glm::vec3 const& p) {
		for (int i=0; i<3; i++) {
			if (p[i] < vMin[i])
				vMin[i] = p[i];
			else if (p[i] > vMax[i])
				vMax[i] = p[i];
		}
	}

	// create a new AABB that is expanded to include the new point
	AABB expanded(glm::vec3 const& p) const {
		AABB a(*this);
		a.expand(p);
		return a;
	}

	// expands the AABB to include all the points inside the other AABB as well (reunion)
	void expand(AABB const& x) {
		for (int i=0; i<3; i++) {
			if (vMin[i] > x.vMin[i])
				vMin[i] = x.vMin[i];
			if (vMax[i] < x.vMax[i])
				vMax[i] = x.vMax[i];
		}
	}

	// creates a new ABB that is expanded to include all points inside the other AABB as well (reunion)
	AABB expanded(AABB const& x) const {
		AABB a(*this);
		a.expand(x);
		return a;
	}

	// replaces this AABB with the intersection of it and another AABB
	void intersect(AABB const& x) {
		if (x.vMin.x >= vMax.x ||
			x.vMax.x <= vMin.x ||
			x.vMin.y >= vMax.y ||
			x.vMax.y <= vMin.y ||
			x.vMin.z >= vMax.z ||
			x.vMin.z <= vMin.z)
			*this = empty();
		else
			*this = AABB(glm::vec3(max(vMin.x, x.vMin.x), max(vMin.y, x.vMin.y), max(vMin.z, x.vMin.z)),
				glm::vec3(min(vMax.x, x.vMax.x), min(vMax.y, x.vMax.y), min(vMax.z, x.vMax.z)));
	}

	// creates a new AABB that is the result of intersection of this AABB with another
	AABB intersected(AABB const& x) const {
		AABB a(*this);
		a.intersect(x);
		return a;
	}

	// offsets this AABB by a given amount
	void offset(glm::vec3 const& o) {
		vMin += o;
		vMax += o;
	}

	// creates a new offseted AABB from this one
	AABB offseted(glm::vec3 const& o) const {
		AABB a(*this);
		a.offset(o);
		return a;
	}

	bool intersectsSphere(glm::vec3 const& c, float r) const {
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
