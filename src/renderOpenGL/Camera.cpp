/*
 * Camera.cpp
 *
 *  Created on: Nov 9, 2014
 *      Author: bog
 */

#include <boglfw/renderOpenGL/Camera.h>
#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/math/math3D.h>
#include <boglfw/utils/log.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

Camera::Camera(Viewport* vp)
	: pViewport_(vp)
	, fov_(PI/3)
	, matView_(1)
	, matProj_(1)
	, position_(0)
	, direction_(0, 0, 1)
{
	setOrthoZoom(100);
}

Camera::~Camera() {
}

glm::vec3 Camera::localX() const {
	return vec4xyz(m4row(matView_, 0));
}

glm::vec3 Camera::localY() const {
	return vec4xyz(m4row(matView_, 1));
}

void Camera::setFOV(float fov) {
	fov_ = fov;
	updateProj();
}

void Camera::setOrthoZoom(float zoom) {
	zoomLevel_ = zoom;
	float width = pViewport_->width() / zoomLevel_;
	float height = pViewport_->height() / zoomLevel_;
	orthoSize_ = { width, height };
	fov_ = 0;
	updateProj();
}

void Camera::setOrtho(glm::vec2 size) {
	orthoSize_ = size;
	fov_ = 0;
	zoomLevel_ = pViewport_->width() / orthoSize_.x;
	updateProj();
}

void Camera::move(glm::vec3 delta) {
	position_ += delta;
	updateView();
}

void Camera::moveTo(glm::vec3 where) {
	position_ = where;
	updateView();
}

void Camera::lookAt(glm::vec3 where, glm::vec3 up) {
	direction_ = glm::normalize(where - position_);
	up_ = up;
	updateView();
}

void Camera::orbit(glm::vec3 center, glm::quat rotation, bool lookTowardCenter) {
	glm::vec3 offset = position_ - center;
	glm::vec3 newOffset = vec4xyz(rotation * glm::vec4(offset, 1.f));
	position_ = center + newOffset;
	if (lookTowardCenter)
		direction_ = glm::normalize(-newOffset);
	updateView();
}

void Camera::mirror(glm::vec4 plane) {
	glm::vec3 N{plane.x, plane.y, plane.z};
	float posDist = glm::dot(N, position_) + plane.w;
	position_ -= N * 2.f * posDist;
	direction_ -= N * 2.f * glm::dot(N, direction_);
	up_ -= N * 2.f * glm::dot(N, up_);
	updateView();
}

void Camera::updateView() {
	matView_ = glm::lookAtLH(position_, position_ + direction_, up_);
}

void Camera::setZPlanes(float zNear, float zFar) {
	zNear_ = zNear;
	zFar_ = zFar;
	updateProj();
}

void Camera::updateProj() {
	if (fov_ == 0) {
		// set ortho
		matProj_ = glm::orthoLH(-orthoSize_.x * 0.5f, orthoSize_.x * 0.5f, -orthoSize_.y * 0.5f, orthoSize_.y * 0.5f, zNear_, zFar_);
	} else {
		// set perspective
		matProj_ = glm::perspectiveFovLH(fov_, (float)pViewport_->width(), (float)pViewport_->height(), zNear_, zFar_);
	}
}
