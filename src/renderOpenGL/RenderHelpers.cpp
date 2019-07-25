#include <boglfw/renderOpenGL/RenderHelpers.h>
#include <boglfw/renderOpenGL/Shape3D.h>
#include <boglfw/renderOpenGL/Shape2D.h>
#include <boglfw/renderOpenGL/MeshRenderer.h>
#include <boglfw/renderOpenGL/GLText.h>
#include <boglfw/renderOpenGL/PictureDraw.h>
#include <boglfw/renderOpenGL/glToolkit.h>

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
	checkGLError("before RenderHelpers::flushAll()");
	if (!config_.disableShape2D)
		Shape2D::get()->flush();
	checkGLError("after Shape2D::flush()");
	if (!config_.disableShape3D)
		Shape3D::get()->flush();
	checkGLError("after Shape3D::flush()");
	if (!config_.disableMeshRenderer)
		MeshRenderer::get()->flush();
	checkGLError("after MeshRenderer::flush()");
	if (!config_.disableGLText)
		GLText::get()->flush();
	checkGLError("after GLText::flush()");
	if (!config_.disablePictureDraw)
		PictureDraw::get()->flush();
	checkGLError("after PictureDraw::flush()");
}
