/*
 * MeshRenderer.cpp
 *
 *  Created on: Apr 25, 2017
 *      Author: bog
 */

#include <boglfw/renderOpenGL/MeshRenderer.h>
#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/Mesh.h>
#include <boglfw/renderOpenGL/shader.h>
#include <boglfw/renderOpenGL/Camera.h>
#include <boglfw/renderOpenGL/glToolkit.h>
#include <boglfw/renderOpenGL/RenderHelpers.h>
#include <boglfw/utils/log.h>

#include <boglfw/utils/assert.h>

#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h>
#include <GL/gl.h>


static MeshRenderer* instance = nullptr;

void MeshRenderer::init() {
	instance = new MeshRenderer();
}

MeshRenderer* MeshRenderer::get() {
	assertDbg(instance && "must be initialized first!");
	return instance;
}

void MeshRenderer::unload() {
	delete instance;
	instance = nullptr;
}

MeshRenderer::MeshRenderer() {
	LOGPREFIX("MeshRenderer");
	Shaders::createProgram("data/shaders/mesh.vert", "data/shaders/mesh-texture.frag", [this](unsigned id) {
		meshShaderProgram_ = id;
		if (meshShaderProgram_ == 0) {
			throw std::runtime_error("Unable to load mesh shaders!!");
		}
		indexPos_ = glGetAttribLocation(meshShaderProgram_, "vPos");
		indexNorm_ = glGetAttribLocation(meshShaderProgram_, "vNormal");
		indexUV1_ = glGetAttribLocation(meshShaderProgram_, "vUV1");
		indexColor_ = glGetAttribLocation(meshShaderProgram_, "vColor");
		indexMatPVW_ = glGetUniformLocation(meshShaderProgram_, "mPVW");
		checkGLError("getAttribs");
	});
}

MeshRenderer::~MeshRenderer() {
	glDeleteProgram(meshShaderProgram_);
}

void MeshRenderer::renderMesh(Mesh& mesh, glm::mat4 worldTransform) {
	renderQueue_.push_back(meshRenderData(&mesh, worldTransform));
}

void MeshRenderer::flush() {
	LOGPREFIX("MeshRenderer::flush");

	if (!meshShaderProgram_)
		return;

	Viewport* vp = RenderHelpers::getActiveViewport();
	if (!vp) {
		assertDbg(!!!"No viewport is currently rendering!");
		return;
	}

	unsigned nMeshes = renderQueue_.size();
	if (!nMeshes)
		return;

	glUseProgram(meshShaderProgram_);

	auto matPV = vp->camera().matProjView();

	for (unsigned i=0; i<nMeshes; i++) {
		auto &m = renderQueue_[i];
		auto matPVW = matPV * m.wldTransform_;
		glUniformMatrix4fv(indexMatPVW_, 1, GL_FALSE, glm::value_ptr(matPVW));
		checkGLError("mPVW uniform setup");

		glBindVertexArray(m.pMesh_->getVAO());
		if (m.pMesh_->vertexAttribsProgramBinding_ != meshShaderProgram_) {
			glBindBuffer(GL_ARRAY_BUFFER, m.pMesh_->getVBO());
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.pMesh_->getIBO());
			glEnableVertexAttribArray(indexPos_);
			glEnableVertexAttribArray(indexNorm_);
			glEnableVertexAttribArray(indexUV1_);
			glEnableVertexAttribArray(indexColor_);
			glVertexAttribPointer(indexPos_, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::s_Vertex), (void*)offsetof(Mesh::s_Vertex, position));
			glVertexAttribPointer(indexNorm_, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::s_Vertex), (void*)offsetof(Mesh::s_Vertex, normal));
			glVertexAttribPointer(indexUV1_, 2, GL_FLOAT, GL_FALSE, sizeof(Mesh::s_Vertex), (void*)offsetof(Mesh::s_Vertex, UV1));
			glVertexAttribPointer(indexColor_, 4, GL_FLOAT, GL_FALSE, sizeof(Mesh::s_Vertex), (void*)offsetof(Mesh::s_Vertex, color));
			m.pMesh_->vertexAttribsProgramBinding_ = meshShaderProgram_;
			checkGLError("attrib arrays setup");
		}
		// decide what to draw:
		unsigned drawMode = 0;
		switch (m.pMesh_->getRenderMode()) {
			case Mesh::RENDER_MODE_POINTS:
				drawMode = GL_POINTS; break;
			case Mesh::RENDER_MODE_LINES:
				drawMode = GL_LINES; break;
			case Mesh::RENDER_MODE_TRIANGLES:
			case Mesh::RENDER_MODE_TRIANGLES_WIREFRAME:
				drawMode = GL_TRIANGLES; break;
			default:
				assertDbg(false && "Unknown mesh draw mode!");
		}
		if (m.pMesh_->getRenderMode() == Mesh::RENDER_MODE_TRIANGLES_WIREFRAME || m.pMesh_->getRenderMode() == Mesh::RENDER_MODE_LINES) {
			glLineWidth(2.f);
		}
		if (m.pMesh_->getRenderMode() == Mesh::RENDER_MODE_TRIANGLES_WIREFRAME) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		glDrawElements(drawMode, m.pMesh_->getElementsCount(), GL_UNSIGNED_SHORT, 0);
		checkGLError("mesh draw");
		glBindVertexArray(0);
		if (m.pMesh_->getRenderMode() == Mesh::RENDER_MODE_TRIANGLES_WIREFRAME || m.pMesh_->getRenderMode() == Mesh::RENDER_MODE_LINES) {
			glLineWidth(1.f);
		}
		if (m.pMesh_->getRenderMode() == Mesh::RENDER_MODE_TRIANGLES_WIREFRAME) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}

	// purge cached data:
	renderQueue_.clear();
}
