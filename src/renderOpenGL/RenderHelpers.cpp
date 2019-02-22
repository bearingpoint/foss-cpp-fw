#include <boglfw/renderOpenGL/RenderHelpers.h>
#include <boglfw/renderOpenGL/Shape3D.h>
#include <boglfw/renderOpenGL/Shape2D.h>
#include <boglfw/renderOpenGL/MeshRenderer.h>
#include <boglfw/renderOpenGL/GLText.h>

void RenderHelpers::load(Config config) {
	Shape3D::init();
	MeshRenderer::init();
	Shape2D::init();
	GLText::init(config.fontPath);
}

void RenderHelpers::unload() {
	Shape3D::unload();
	Shape2D::unload();
	MeshRenderer::unload();
	GLText::unload();
}

void RenderHelpers::flushAll() {
	Shape2D::flush();
	Shape3D::flush();
	MeshRenderer::flush();
	GLText::flush();
}
