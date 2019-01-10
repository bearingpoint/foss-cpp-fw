/*
 * CameraController.h
 *
 *  Created on: May 7, 2017
 *      Author: bog
 */

#ifndef ENTITIES_CAMERACONTROLLER_H_
#define ENTITIES_CAMERACONTROLLER_H_

#include <boglfw/entities/Entity.h>
#include <boglfw/entities/enttypes.h>
#include <boglfw/utils/path-lerper.h>
#include <boglfw/math/aabb.h>

#include <glm/vec3.hpp>

#include <vector>
#include <memory>

class Camera;

class CameraController: public Entity {
public:
	CameraController(Camera* target);
	virtual ~CameraController();

	unsigned getEntityType() const override { return EntityTypes::CAMERA_CTRL; }
	// these flags MUST NOT change during the life time of the object, or else UNDEFINED BEHAVIOUR
	FunctionalityFlags getFunctionalityFlags() const override { return FunctionalityFlags::UPDATABLE; }
	virtual aabb getAABB(bool requirePrecise=false) const override { return aabb(); }

	void update(float dt) override;

	// starts a back-and-forth movement between p1 and p2. starts from the current position toward p1 first.
	void startBackAndForth(glm::vec3 p1, glm::vec3 p2, glm::vec3 lookAtTarget, float speed);
	// appends a new checkpoint to the camera path node list (if currently stopped, it will start moving toward this checkpoint)
	void addNextCheckpoint(glm::vec3 p, glm::vec3 lookAtTarget, float speed);
	// stops camera motion;
	void stop();
	
	// attaches the controller to the given entity; camera will follow this entity precisely, both in position and orientation.
	// the camera is attached at <offset> from the entity's origin; <offset> is expressed in entity's local space.
	// To detach the controller from the entity, call this again with nullptr; the camera will remain in its last position
	void attachToEntity(std::weak_ptr<Entity> ent, glm::vec3 offset);

	struct CameraNode {
		glm::vec3 position;
		glm::vec3 lookAtTarget;

		CameraNode operator*(float factor) const {
			return CameraNode {
				position * factor, lookAtTarget * factor
			};
		}

		CameraNode operator+(CameraNode v) const {
			return CameraNode {
				position + v.position, lookAtTarget + v.lookAtTarget
			};
		}

		// distance function
		float operator-(CameraNode v) const {
			return (position - v.position).length();
		}
	};

	PathLerper<CameraNode>& pathLerper() { return pathLerper_; }

private:
	Camera* camera_ = nullptr;
	PathLerper<CameraNode> pathLerper_;
	std::weak_ptr<Entity> attachedEntity_;
	glm::vec3 attachOffset_;
};

#endif /* ENTITIES_CAMERACONTROLLER_H_ */
