/*
 * Camera.h
 *
 *  Created on: Nov 9, 2014
 *      Author: bog
 */

#ifndef RENDEROPENGL_CAMERA_H_
#define RENDEROPENGL_CAMERA_H_

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

class Viewport;

class Camera {
public:
	Camera(Viewport* vp);
	virtual ~Camera();

	const glm::mat4& matView() const { return matView_; }
	const glm::mat4& matProj() const { return matProj_; }
	const glm::mat4 matProjView() const { return matProj_ * matView_; }

	float getOrthoZoom() { return zoomLevel_; } // how many pixels per meter?
	void setOrthoZoom(float zoom);

	// returns the camera position in world space
	const glm::vec3& position() const { return position_; }
	// returns the camera look direction in world space
	const glm::vec3& direction() const { return direction_; }
	// returns the local Y axis (up) vector of the camera, expressed in world space
	glm::vec3 localY() const;
	// returns the local X axis (right) vector of the camera, expressed in world space
	glm::vec3 localX() const;

	void move(glm::vec3 delta);
	void moveTo(glm::vec3 where);
	void lookAt(glm::vec3 where, glm::vec3 up = glm::vec3{0.f, 1.f, 0.f});
	void transformView(glm::mat4 rTrans);
	void setViewTransform(glm::mat4 aTrans);
	// orbits the camera around a central point, with a rotation quaternion expressed in local camera coordinates
	// if [lookTowardCenter] is true, the camera is also redirected to look toward the center point,
	// otherwise its original orientation is kept
	void orbit(glm::vec3 center, glm::quat rotation, bool lookTowardCenter=true);
	void mirror(glm::vec4 mirrorPlane);
	void setZPlanes(float zNear, float zFar);
	float FOV() const { return fov_; }
	void setFOV(float fov);
	void setOrtho(glm::vec2 size); // sets ortho projection size {width, height} in world units
	glm::vec4 getOrthoRect() const {
		return {
			position_.x - orthoSize_.x * 0.5f,	// left
			position_.y - orthoSize_.y * 0.5f,	// bottom
			position_.x + orthoSize_.x * 0.5f,	// right
			position_.y + orthoSize_.y * 0.5f,  // top
		};
	}

protected:
	Viewport* pViewport_;
	float fov_;
	float zoomLevel_;
	float zNear_ = 0.5f;
	float zFar_ = 100.f;
	glm::mat4 matView_;
	glm::mat4 matProj_;
	glm::vec3 position_ {0.f, 0.f, -1.f};
	glm::vec3 direction_ {0.f, 0.f, 1.f};
	glm::vec3 up_ {0.f, 1.f, 0.f};
	glm::vec2 orthoSize_; // in world units

	void updateView();
	void updateProj();

	friend class Viewport;
};

#endif /* RENDEROPENGL_CAMERA_H_ */
