#include <boglfw/renderOpenGL/RenderHelpers.h>
#include <boglfw/renderOpenGL/Shape3D.h>
#include <boglfw/renderOpenGL/Shape2D.h>
#include <boglfw/renderOpenGL/MeshRenderer.h>
#include <boglfw/renderOpenGL/GLText.h>
#include <boglfw/renderOpenGL/PictureDraw.h>

Viewport* RenderHelpers::pActiveViewport = nullptr;
RenderHelpers::Config RenderHelpers::config_;

void RenderHelpers::load(Config config) {
	config_ = config;
	
	if (!config.disableShape3D)
		Shape3D::init();
	if (!config.disableMeshRenderer)
		MeshRenderer::init();
	if (!config.disableShape2D)
		Shape2D::init();
	if (!config.disableGLText)
		GLText::init(config.fontPath.c_str());
	if (!config.disablePictureDraw)
		PictureDraw::init();
}

void RenderHelpers::unload() {
	if (!config_.disableShape3D)
		Shape3D::unload();
	if (!config_.disableShape2D)
		Shape2D::unload();
	if (!config_.disableMeshRenderer)
		MeshRenderer::unload();
	if (!config_.disableGLText)
		GLText::unload();
	if (!config_.disablePictureDraw)
		PictureDraw::unload();
}

void RenderHelpers::flushAll() {
	if (!config_.disableShape2D)
		Shape2D::get()->flush();
	if (!config_.disableShape3D)
		Shape3D::get()->flush();
	if (!config_.disableMeshRenderer)
		MeshRenderer::get()->flush();
	if (!config_.disableGLText)
		GLText::get()->flush();
	if (!config_.disablePictureDraw)
		PictureDraw::get()->flush();
}
