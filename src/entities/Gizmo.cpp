#include <boglfw/entities/Gizmo.h>

#include <boglfw/renderOpenGL/MeshRenderer.h>
#include <boglfw/math/aabb.h>

Gizmo::Gizmo(glm::mat4 transform, float axisLength)
	: axisLength_(axisLength)
{
	mesh_.createGizmo(axisLength);
}

Gizmo::~Gizmo()
{
}

void Gizmo::draw(Viewport* vp) {
	MeshRenderer::get()->renderMesh(mesh_, transform_.glMatrix());
}
