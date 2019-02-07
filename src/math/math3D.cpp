#include <boglfw/math/math3D.h>

#include <algorithm>

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

glm::vec2 rayIntersectBox(float length, float width, float direction) {
	float hw = width * 0.5f, hl = length * 0.5f;	// half width and length
	// bring the angle between [-PI, +PI]
	float phiQ = atanf(width/length);
	float relativeAngle = limitAngle(direction, 2*PI-phiQ);
	if (relativeAngle < phiQ) {
		// front edge
		glm::vec2 ret(hl, sinf(relativeAngle) * hw);
		return ret;
	} else if (relativeAngle < PI-phiQ  || relativeAngle > PI+phiQ) {
		// left or right edge
		glm::vec2 ret(cosf(relativeAngle) * hl, relativeAngle < PI ? hw : -hw);
		return ret;
	} else {
		// back edge
		glm::vec2 ret(-hl, sinf(relativeAngle) * hw);
		return ret;
	}
}

bool angleSpanOverlap(float angle1, float span1, float angle2, float span2, bool sweepPositive, float &outMargin) {
	// will set outMargin to negative if overlap, or positive shortest gap around element if no overlap
	// move angle1 in origin for convenience:
	angle2 = limitAngle(angle2 - angle1, PI);
	angle1 = 0;
	float a1p = span1*0.5f, a1n = -span1*0.5f;
	float a2p = angle2 + span2*0.5f, a2n = angle2 - span2*0.5f;
	if (a2p*a2n >= 0) {
		// both ends of span2 are on the same side of angle 1
		if (a2p > 0) { // both on the positive side
			outMargin = min(a2p, a2n) - a1p;
			if (!sweepPositive)
				outMargin = 2*PI-outMargin-span1-span2;
		} else {// both on the negative side
			outMargin = a1n - max(a2n, a2p);
			if (sweepPositive)
				outMargin = 2*PI-outMargin-span1-span2;
		}
	} else { // ends of span2 are on different sides of angle 1
		float d1 = a2n-a1p, d2 = a1n-a2p;
		outMargin = angle2 > 0 ? d1 : d2; // the smallest distance between the spans (positive) or the greatest overlap (negative)
		if (sweepPositive == (angle2<=0))
			outMargin = 2*PI-outMargin-span1-span2;
	}

	return outMargin < 0;
}

glm::mat4 buildMatrix(glm::vec3 right, glm::vec3 up, glm::vec3 front, glm::vec3 translation) {
	return glm::mat4 {
		right.x,	right.y,	right.z,	translation.x,
		up.x,		up.y,		up.z,		translation.y,
		front.x,	front.y,	front.z,	translation.z,
		0.f, 		0.f, 		0.f, 		1.f
	};
}

glm::mat4 buildMatrixFromOrientation(glm::vec3 position, glm::vec3 direction, glm::vec3 up) {
	direction = glm::normalize(direction);
	glm::vec3 right = glm::normalize(glm::cross(up, direction));
	up = glm::cross(direction, right);
	return buildMatrix(right, up, direction, position);
}

bool rayIntersectTri(glm::vec3 const& start, glm::vec3 const& dir,
	glm::vec3 const& p1, glm::vec3 const& p2, glm::vec3 const&p3,
	glm::vec3 &outIntersectionPoint) {

	glm::vec3 p1p3 = p3 - p1;
	glm::vec3 p1p2 = p2 - p1;
	glm::vec3 triNorm = glm::normalize(glm::cross(p1p2, p1p3));				// triangle normal
	float d = -glm::dot(triNorm, p1);										// d component of plane equation (triNorm has a,b,c)
	float t = (-d - glm::dot(triNorm, start)) / glm::dot(triNorm, dir);		// ray parameter t for intersection between ray and triangle plane
	if (t < 0.f)															// t<0 means intersection point is behind start
		return false;
	outIntersectionPoint = start + dir * t;									// this is the point in the triangle's plane where the ray hits
	glm::vec3 p1P = outIntersectionPoint - p1;
	// now check if the point is within the triangle
	float d00 = glm::dot(p1p3, p1p3);
	float d01 = glm::dot(p1p2, p1p3);
	float d02 = glm::dot(p1p3, p1P);
	float d11 = glm::dot(p1p2, p1p2);
	float d12 = glm::dot(p1p2, p1P);
	float invDenom = 1.f / (d00 * d11 - d01 * d01);
	// compute barycentric coords:
	float u = (d11 * d02 - d01 * d12) * invDenom;
	float v = (d00 * d12 - d01 * d02) * invDenom;

	return u >= 0 && v >= 0 && (u+v) <= 1;
}
