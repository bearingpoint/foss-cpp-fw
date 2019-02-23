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
}

Box::~Box() {
}

void Box::update(float dt) {
	//body_.update(dt);
}

void Box::draw(RenderContext const& ctx) {
	MeshRenderer::get()->renderMesh(mesh_, transform_.glMatrix()/*body_.getTransformation(physics::DynamicBody::TransformSpace::World)*/);
}
