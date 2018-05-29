/*
 * Cube.cpp
 *
 *  Created on: Apr 24, 2017
 *      Author: bog
 */

#include <boglfw/entities/Box.h>
#include <boglfw/renderOpenGL/MeshRenderer.h>

Box::Box(float width, float height, float depth, glm::vec3 offset) {
	mesh_.createBox(offset, width, height, depth);
}

Box::~Box() {
}

void Box::update(float dt) {
	//body_.update(dt);
}

void Box::draw(Viewport* vp) {
	MeshRenderer::get()->renderMesh(mesh_, glm::mat4(1)/*body_.getTransformation(physics::DynamicBody::TransformSpace::World)*/);
}
