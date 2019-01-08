#include <boglfw/entities/Gizmo.h>

#include <boglfw/renderOpenGL/MeshRenderer.h>
#include <boglfw/math/aabb.h>

Gizmo::Gizmo(glm::mat4 transform, float axisLength)
	: transform_(transform)
	, axisLength_(axisLength)
{
	mesh_.createGizmo(axisLength);
}

Gizmo::~Gizmo()
{
}

void Gizmo::draw(Viewport* vp) {
	MeshRenderer::get()->renderMesh(mesh_, transform_);
}

aabb Gizmo::getAABB(bool requirePrecise) const {
	glm::vec3 tl = m4Translation(transform_);
	// just a rough estimate that doesn't take rotation into account
	return aabb(tl + glm::vec3{-axisLength_, -axisLength_, -axisLength_}, tl + glm::vec3{axisLength_, axisLength_, axisLength_});
}