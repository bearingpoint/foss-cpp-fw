#ifndef GIZMO_H
#define GIZMO_H

#include <boglfw/entities/Entity.h>
#include <boglfw/entities/enttypes.h>
#include <boglfw/renderOpenGL/Mesh.h>

#include <glm/mat4x4.hpp>

class Gizmo : public Entity
{
public:
	virtual FunctionalityFlags getFunctionalityFlags() const override { return FunctionalityFlags::DRAWABLE; }
	virtual unsigned getEntityType() const override { return EntityTypes::GIZMO; }
	virtual ~Gizmo() override;

	Gizmo(glm::mat4 transform, float axisLength = 1.f);

	virtual void draw(RenderContext const& ctx) override;

private:
	float axisLength_;
	Mesh mesh_;
};

#endif // GIZMO_H
