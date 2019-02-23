/*
 * Entity.h
 *
 *  Created on: Jan 21, 2015
 *      Author: bog
 */

#ifndef ENTITIES_ENTITY_H_
#define ENTITIES_ENTITY_H_

#include <boglfw/utils/bitFlags.h>
#include <boglfw/math/transform.h>

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

#include <atomic>

class RenderContext;
class BinaryStream;
struct AABB;

class Entity {
public:
	virtual ~Entity();

	enum class FunctionalityFlags {
		NONE			= 0,
		DONT_CARE		= 0,
		DRAWABLE		= 1,
		UPDATABLE		= 2,
		SERIALIZABLE	= 4,
	};

	// these flags MUST NOT change during the life time of the object, or else UNDEFINED BEHAVIOUR
	virtual FunctionalityFlags getFunctionalityFlags() const { return FunctionalityFlags::NONE; }

	virtual void update(float dt) {}
	virtual void draw(RenderContext const& ctx) {}
	virtual void serialize(BinaryStream &stream) const;
	virtual int getSerializationType() const;
	virtual unsigned getEntityType() const = 0;

	// return the world transormation of the entity
	virtual Transform& getTransform() { return transform_; }
	virtual const Transform& getTransform() const { return transform_; }

	// return the AABB that contains this entity
	// the default implementation returns a 1mx1mx1m AABB centered around the entitiy's transform origin
	virtual AABB getAABB() const;

	void destroy();
	bool isZombie() const { return markedForDeletion_.load(std::memory_order_acquire); }

protected:
	Entity() = default;

	Transform transform_;

private:
	std::atomic<bool> markedForDeletion_ {false};
	bool managed_ = false;
	friend class World;
};

#endif /* ENTITIES_ENTITY_H_ */
