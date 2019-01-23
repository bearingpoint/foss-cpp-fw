/*
 * MeshRenderer.cpp
 *
 *  Created on: Apr 25, 2017
 *      Author: bog
 */

#include <boglfw/renderOpenGL/MeshRenderer.h>
#include <boglfw/renderOpenGL/Renderer.h>
#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/Mesh.h>
#include <boglfw/renderOpenGL/shader.h>
#include <boglfw/renderOpenGL/Camera.h>
#include <boglfw/renderOpenGL/glToolkit.h>
#include <boglfw/utils/log.h>

#include <boglfw/utils/assert.h>

#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h>
#include <GL/gl.h>


static MeshRenderer* instance = nullptr;

void MeshRenderer::init(Renderer* renderer) {
	instance = new MeshRenderer(renderer);
}

MeshRenderer* MeshRenderer::get() {
	assertDbg(instance && "must be initialized first!");
	return instance;
}

void MeshRenderer::unload() {
	delete instance;
	instance = nullptr;
}

MeshRenderer::MeshRenderer(Renderer* renderer) {
	LOGPREFIX("MeshRenderer::ctor");
	renderer->registerRenderable(this);
	meshShaderProgram_ = Shaders::createProgram("data/shaders/mesh.vert", "data/shaders/mesh-texture.frag");
	if (meshShaderProgram_ == 0) {
		throw std::runtime_error("Unable to load mesh shaders!!");
	}
	indexPos_ = glGetAttribLocation(meshShaderProgram_, "vPos");
	indexNorm_ = glGetAttribLocation(meshShaderProgram_, "vNormal");
	indexUV1_ = glGetAttribLocation(meshShaderProgram_, "vUV1");
	indexColor_ = glGetAttribLocation(meshShaderProgram_, "vColor");
	indexMatPVW_ = glGetUniformLocation(meshShaderProgram_, "mPVW");
	checkGLError("getAttribs");
}

MeshRenderer::~MeshRenderer() {
	glDeleteProgram(meshShaderProgram_);
}

void MeshRenderer::renderMesh(Mesh& mesh, glm::mat4 worldTransform) {
	renderQueue_.push_back(meshRenderData(&mesh, worldTransform));
}

void MeshRenderer::render(Viewport* vp, unsigned batchId) {
	LOGPREFIX("MeshRenderer::render");
	assertDbg(batchId < batches_.size());
	
	unsigned nMeshes = batchId == batches_.size() - 1 ? renderQueue_.size() - batches_.back()
		: batches_[batchId+1] - batches_[batchId];
	if (!nMeshes)
		return;
	
	glUseProgram(meshShaderProgram_);

	auto matPV = vp->camera()->matProjView();

	for (unsigned i=0; i<nMeshes; i++) {
		auto &m = renderQueue_[batches_[batchId] + i];
		auto matPVW = matPV * m.wldTransform_;
		glUniformMatrix4fv(indexMatPVW_, 1, GL_FALSE, glm::value_ptr(matPVW));
		checkGLError("mPVW uniform setup");

		glBindVertexArray(m.pMesh_->getVAO());
		if (!m.pMesh_->vertexAttribsSet_) {
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
			m.pMesh_->vertexAttribsSet_ = true;
			checkGLError("attrib arrays setup");
		}
		// decide what to draw:
		unsigned drawMode = 0;
		switch (m.pMesh_->mode_) {
			case Mesh::RENDER_MODE_POINTS:
				drawMode = GL_POINTS; break;
			case Mesh::RENDER_MODE_LINES:
				drawMode = GL_LINES; break;
			case Mesh::RENDER_MODE_TRIANGLES:
			case Mesh::RENDER_MODE_TRIANGLES_WIREFRAME:
				drawMode = GL_TRIANGLES; break;
			default:
				assert(false && "Unknown mesh draw mode!");
		}
		if (m.pMesh_->mode_ == Mesh::RENDER_MODE_TRIANGLES_WIREFRAME) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		glDrawElements(drawMode, m.pMesh_->getElementsCount(), GL_UNSIGNED_SHORT, 0);
		checkGLError("mesh draw");
		glBindVertexArray(0);
		if (m.pMesh_->mode_ == Mesh::RENDER_MODE_TRIANGLES_WIREFRAME) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}
}

void MeshRenderer::startBatch() {
	batches_.push_back(renderQueue_.size());
}

void MeshRenderer::setupFrameData() {
	// nothing
	// TODO -> if we have dynamic meshes, then call update on them or something
}

void MeshRenderer::purgeRenderQueue() {
	renderQueue_.clear();
	batches_.clear();
}
