/*
 * Cube.h
 *
 *  Created on: Apr 24, 2017
 *      Author: bog
 */

#ifndef ENTITIES_BOX_H_
#define ENTITIES_BOX_H_

#include <boglfw/entities/Entity.h>
#include <boglfw/entities/enttypes.h>
#include <boglfw/renderOpenGL/Mesh.h>
#include <boglfw/math/aabb.h>
//#include "../physics/DynamicBody.h"

#include <glm/mat4x4.hpp>

class Box: public Entity {
public:
	Box(float width, float height, float depth, glm::vec3 centerOffset = glm::vec3{0.f});
	virtual ~Box();

	unsigned getEntityType() const override { return EntityTypes::BOX; }
	FunctionalityFlags getFunctionalityFlags() const override { return FunctionalityFlags::DRAWABLE | FunctionalityFlags::UPDATABLE; }

	void update(float dt) override;
	void draw(Viewport* vp) override;
	
	virtual aabb getAABB(bool requirePrecise=false) const override;

	//physics::DynamicBody* body() { return &body_; }

private:
	Mesh mesh_;
	glm::mat4 transform_ { 1.f };
	aabb modelAABB_;
	//physics::DynamicBody body_;
};

#endif /* ENTITIES_BOX_H_ */
