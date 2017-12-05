/*
 * Entity.cpp
 *
 *  Created on: Jan 21, 2015
 *      Author: bog
 */

#include <boglfw/entities/Entity.h>
#include <boglfw/World.h>
#include <boglfw/utils/assert.h>
#include <boglfw/utils/log.h>

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

Entity::~Entity() {
	assertDbg((!managed_ || markedForDeletion_) && "You should never call delete on a managed Entity directly! (use destroy() instead)");
}

void Entity::destroy() {
	bool expect = false;
	if (!markedForDeletion_.compare_exchange_strong(expect, true, std::memory_order_acq_rel, std::memory_order_relaxed)) {
		return;
	}
	if (managed_)
		World::getInstance()->destroyEntity(this);
	else
		delete this;
}

void Entity::serialize(BinaryStream &stream) { assertDbg(false && "forgot to override this?"); }

SerializationObjectTypes Entity::getSerializationType() {
	assertDbg(false && "forgot to override this?");
	return (SerializationObjectTypes)0;
}

glm::vec2 Entity::getPosition() {
	return vec3xy(getWorldTransform());
}

