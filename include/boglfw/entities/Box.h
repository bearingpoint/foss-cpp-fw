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
//#include "../physics/DynamicBody.h"

class Box: public Entity {
public:
	Box(float width, float height, float depth, glm::vec3 offset = glm::vec3(0));
	virtual ~Box();

	unsigned getEntityType() const override { return EntityTypes::BOX; }
	FunctionalityFlags getFunctionalityFlags() const override { return FunctionalityFlags::DRAWABLE | FunctionalityFlags::UPDATABLE; }

	void update(float dt) override;
	void draw(Viewport* vp) override;

	//physics::DynamicBody* body() { return &body_; }

private:
	Mesh mesh_;
	//physics::DynamicBody body_;
};

#endif /* ENTITIES_BOX_H_ */
