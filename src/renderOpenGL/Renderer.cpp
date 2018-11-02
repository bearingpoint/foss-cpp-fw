/*
 * Renderer.cpp
 *
 *  Created on: Nov 2, 2014
 *      Author: bog
 */

#include <boglfw/renderOpenGL/Renderer.h>
#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/Shape2D.h>
#include <boglfw/renderOpenGL/Shape3D.h>
#include <boglfw/renderOpenGL/GLText.h>
#include <boglfw/renderOpenGL/MeshRenderer.h>
#include <boglfw/utils/drawable.h>
#include <boglfw/utils/assert.h>

#include <GL/gl.h>

#include <stdexcept>

Renderer::~Renderer() {
}

Renderer::Renderer(int winW, int winH)
	: winW_(winW), winH_(winH)
{
	Shape3D::init(this);
	MeshRenderer::init(this);
	Shape2D::init(this);
	GLText::init(this, "data/fonts/DejaVuSansMono_256_16_8.png", 8, 16, ' ', 22);
}

void Renderer::registerRenderable(IRenderable* r) {
	if (r == nullptr)
		throw new std::invalid_argument("argument cannot be null!");
	renderComponents_.push_back(r);
}

void Renderer::addViewport(std::string name, std::unique_ptr<Viewport> vp) {
	vp->setName(name);
	viewports_[name] = std::move(vp);
}

Viewport* Renderer::getViewport(std::string name) const {
	auto it = viewports_.find(name);
	if (it == viewports_.end()) {
		return nullptr;
	} else
		return it->second.get();
}

std::vector<Viewport*> Renderer::getViewports() const {
	std::vector<Viewport*> ret;
	for (auto &p : viewports_)
		ret.push_back(p.second.get());
	return ret;
}

void Renderer::deleteViewport(std::string const& name) {
	auto it = viewports_.find(name);
	assertDbg(it != viewports_.end() && "non existing viewport");
	viewports_.erase(it);
}

void Renderer::render() {
	for (auto &vp : viewports_) {
		if (!vp.second->isEnabled())
			continue;
		// 1. clear viewport:
		auto vpp = vp.second->position();
		glViewport(vpp.x, vpp.y, vp.second->width(), vp.second->height());
		glClearColor(vp.second->backgroundColor_.r, vp.second->backgroundColor_.g, vp.second->backgroundColor_.b, 0);
		glScissor(vpp.x, vpp.y, vp.second->width(), vp.second->height());
		glEnable(GL_SCISSOR_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glDisable(GL_SCISSOR_TEST);

		// 2. build the render queue for this viewport
		for (auto &d : vp.second->drawList_)
			d.draw(vp.second.get());

		// 3. do the low-level rendering
		for (auto r : renderComponents_) {
			// TODO optimization: have two queues per IRenderable - one common and one per viewport and only rebuild the second
			r->render(vp.second.get());
			r->purgeRenderQueue();
		}
	}
}

void Renderer::unload() {
	for (auto r : renderComponents_) {
		r->purgeRenderQueue();
		r->unload();
	}
}
