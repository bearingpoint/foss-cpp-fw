/*
 * PhysicsBody.h
 *
 *  Created on: Jan 21, 2015
 *      Author: bog
 */

#ifndef WITH_BOX2D
#error "Box2D support has not been enabled and this is a requirement for using the PhysicsBody class"
#endif

#ifndef OBJECTS_PHYSICSBODY_H_
#define OBJECTS_PHYSICSBODY_H_

#include "../utils/Event.h"
#include "../math/box2glm.h"

#include <glm/vec2.hpp>
#include <Box2D/Dynamics/b2Body.h>
#include <functional>

class b2Body;
class Entity;
struct AABB;

struct PhysicsProperties {
	glm::vec2 position;
	float angle;
	bool dynamic;
	glm::vec2 velocity;
	float angularVelocity;

	PhysicsProperties(glm::vec2 position, float angle, bool dynamic, glm::vec2 velocity, float angularVelocity)
		: position(position), angle(angle), dynamic(dynamic), velocity(velocity), angularVelocity(angularVelocity)
	{}

	PhysicsProperties(glm::vec2 pos, float angle)
		: position(pos), angle(angle), dynamic(true), velocity(glm::vec2(0)), angularVelocity(0)
	{}

	PhysicsProperties() : PhysicsProperties(glm::vec2(0), 0) {}

	PhysicsProperties(const PhysicsProperties& o) = default;
};

class PhysicsBody {
public:
	PhysicsBody(int userObjType, void* userPtr, int categFlags, int collisionMask);
	PhysicsBody() : PhysicsBody(0, nullptr, 0, 0) {}
	PhysicsBody(PhysicsBody &&b);
	PhysicsBody(PhysicsBody const& b) = delete;
	virtual ~PhysicsBody();

	void create(PhysicsProperties const &props);
	inline glm::vec2 getPosition() const { return b2Body_ ? b2g(b2Body_->GetPosition()) : glm::vec2{0, 0}; }
	inline float getRotation() const { return b2Body_ ? b2Body_->GetAngle() : 0; }
	inline Entity* getAssociatedEntity() const { assertDbg(getEntityFunc_ != nullptr); return getEntityFunc_(*this); }
	AABB getAABB() const;

	static PhysicsBody* getForB2Body(b2Body* body);

	Event<void(PhysicsBody *other, float impulseMagnitude)> onCollision;
	Event<void(PhysicsBody* caller)> onDestroy;

	// the Box2D body:
	b2Body* b2Body_;
	// the type of object that owns this body
	int userObjectType_;
	// the pointer MUST be set to the object that owns this body (type of object depends on userObjectType_)
	void* userPointer_;
	// this callback MUST be set to a valid function that will return the associated entity of this body
	std::function<Entity*(PhysicsBody const& body)> getEntityFunc_;

	// bit field for the categories that this object belongs to:
	int categoryFlags_;
	// bit mask for what other categories of objects that this collides with should trigger onCollision events on this object
	int collisionEventMask_;
};

#endif /* OBJECTS_PHYSICSBODY_H_ */
