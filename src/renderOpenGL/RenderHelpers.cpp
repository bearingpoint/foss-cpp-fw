#include <boglfw/renderOpenGL/RenderHelpers.h>
#include <boglfw/renderOpenGL/Shape3D.h>
#include <boglfw/renderOpenGL/Shape2D.h>
#include <boglfw/renderOpenGL/MeshRenderer.h>
#include <boglfw/renderOpenGL/GLText.h>
#include <boglfw/renderOpenGL/PictureDraw.h>

Viewport* RenderHelpers::pActiveViewport = nullptr;

void RenderHelpers::load(Config config) {
	Shape3D::init();
	MeshRenderer::init();
	Shape2D::init();
	GLText::init(config.fontPath.c_str());
	PictureDraw::init();
}

void RenderHelpers::unload() {
	Shape3D::unload();
	Shape2D::unload();
	MeshRenderer::unload();
	GLText::unload();
	PictureDraw::unload();
}

void RenderHelpers::flushAll() {
	Shape2D::get()->flush();
	Shape3D::get()->flush();
	MeshRenderer::get()->flush();
	GLText::get()->flush();
	PictureDraw::get()->flush();
}
