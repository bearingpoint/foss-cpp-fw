#ifndef __TRANSFORM_H__
#define __TRANSFORM_H__

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>

struct Transform {
public:
	// get world position
	const glm::vec3& position() const { return pos_; }
	// get world orientation
	const glm::quat& orientation() const { return orient_; }
	// get 4x4 openGL transformation matrix
	const glm::mat4& glMatrix() const;

	// returns the local X axis expressed in world coordinates
	glm::vec3 axisX() const;
	// returns the local Y axis expressed in world coordinates
	glm::vec3 axisY() const;
	// returns the local Z axis expressed in world coordinates
	glm::vec3 axisZ() const;

	// set a new world position for the transform
	void setPosition(glm::vec3 pos);
	// set a new world orientation for the transform
	void setOrientation(glm::quat orient);
	// move the transform by an amount expressed in *WORLD* coordinates
	void moveWorld(glm::vec3 const& delta);
	// move the transform by an amount expressed in *LOCAL* coordinates
	void moveLocal(glm::vec3 const& delta);
	// rotate the transform by a quaternion expressed in *WORLD* coordinates
	void rotateWorld(glm::quat const& rot);
	// rotate the transform by a quaternion expressed in *LOCAL* coordinates
	void rotateLocal(glm::quat const& rot);

private:
	glm::vec3 pos_ {0.f};
	glm::quat orient_ {1.f, 0.f, 0.f, 0.f};
	mutable glm::mat4 glMat_;
	mutable bool matDirty_ = true;

	void updateGLMat() const;
};

#endif // __TRANSFORM_H__
