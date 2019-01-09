/*
 * Cube.cpp
 *
 *  Created on: Apr 24, 2017
 *      Author: bog
 */

#include <boglfw/entities/Box.h>
#include <boglfw/renderOpenGL/MeshRenderer.h>
#include <boglfw/math/math3D.h>

Box::Box(float width, float height, float depth, glm::vec3 centerOffset) {
	mesh_.createBox(centerOffset, width, height, depth);
	glm::vec3 halfSize {width * 0.5f, height * 0.5f, depth * 0.5f};
	modelAABB_.vMin = centerOffset - halfSize;
	modelAABB_.vMax = centerOffset + halfSize;
}

Box::~Box() {
}

void Box::update(float dt) {
	//body_.update(dt);
}

void Box::draw(Viewport* vp) {
	MeshRenderer::get()->renderMesh(mesh_, transform_/*body_.getTransformation(physics::DynamicBody::TransformSpace::World)*/);
}

aabb Box::getAABB(bool requirePrecise) const {
	// TODO properly transform this into world space taking rotation into account
	return modelAABB_.offset(m4Translation(transform_));
}