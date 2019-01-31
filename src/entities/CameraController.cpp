/*
 * CameraController.cpp
 *
 *  Created on: May 7, 2017
 *      Author: bog
 */

#include <boglfw/entities/CameraController.h>
#include <boglfw/renderOpenGL/Camera.h>
#include <boglfw/math/math3D.h>

CameraController::CameraController(Camera* target)
	: camera_(target) {
}

CameraController::~CameraController() {
}

void CameraController::update(float dt) {
	auto attachedSP = attachedEntity_.lock();
	if (attachedSP) {
		auto tr = attachedSP->getTransform();
		glm::vec3 pos = attachOffset_ + m4Translation(tr);
		camera_->moveTo(pos);
		glm::vec3 dir = vec4xyz(m4row(tr, 2));
		glm::vec3 up = vec4xyz(m4row(tr, 1));
		camera_->lookAt(pos + dir, up);
	} else {
		attachedSP.reset();
		pathLerper_.update(dt);
		camera_->moveTo(pathLerper_.value().position);
		camera_->lookAt(pathLerper_.value().lookAtTarget);
	}
}

void CameraController::startBackAndForth(glm::vec3 p1, glm::vec3 p2, glm::vec3 lookAtTarget, float speed) {
	pathLerper_.setPath({
		PathNode<CameraNode>{PathNodeType::vertex, {p1, lookAtTarget}, 0},	//#0
		PathNode<CameraNode>{PathNodeType::vertex, {p2, lookAtTarget}, 0},	//#1
		PathNode<CameraNode>{PathNodeType::redirect, {{0,0,0}, {0,0,0}}, 0}	// -> #0
	});
	pathLerper_.start(speed);
}

void CameraController::addNextCheckpoint(glm::vec3 p, glm::vec3 lookAtTarget, float speed) {
	pathLerper_.appendNode(PathNode<CameraNode>{ PathNodeType::vertex, {p, lookAtTarget}, 0 });
	if (pathLerper_.isStopped())
		pathLerper_.start(speed);
}

void CameraController::stop() {
	pathLerper_.reset();
}

void CameraController::attachToEntity(std::weak_ptr<Entity> ent, glm::vec3 offset) {
	attachedEntity_ = ent;
	attachOffset_ = offset;
	stop();	// stop any path lerping that might have been going on before
}
