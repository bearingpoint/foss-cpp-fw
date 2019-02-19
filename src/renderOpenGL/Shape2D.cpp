/*
 * Shape2D.cpp
 *
 *  Created on: Nov 14, 2014
 *      Author: bogdan
 */
#include <boglfw/renderOpenGL/Shape2D.h>
#include <boglfw/renderOpenGL/Renderer.h>
#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/Camera.h>
#include <boglfw/renderOpenGL/shader.h>
#include <boglfw/renderOpenGL/glToolkit.h>
#include <boglfw/math/math3D.h>
#include <boglfw/utils/tesselate-vec2.h>
#include <boglfw/utils/log.h>
#include <boglfw/utils/arrayContainer.h>
#include <boglfw/perf/marker.h>

#include <glm/mat4x4.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLEW_NO_GLU
#include <GL/glew.h>
#include <stdexcept>

static Shape2D* instance = nullptr;

void Shape2D::init(Renderer* renderer) {
	instance = new Shape2D(renderer);
}

Shape2D* Shape2D::get() {
	assertDbg(instance && "must be initialized first!");
	return instance;
}

Shape2D::Shape2D(Renderer* renderer) {
	renderer->registerRenderable(this);
	glGenVertexArrays(1, &lineVAO_);
	glGenBuffers(1, &lineVBO_);
	glGenBuffers(1, &lineIBO_);
	glGenVertexArrays(1, &triangleVAO_);
	glGenBuffers(1, &triangleVBO_);
	glGenBuffers(1, &triangleIBO_);

	Shaders::createProgram("data/shaders/shape2d.vert", "data/shaders/shape2d.frag", [this](unsigned id) {
		shaderProgram_ = id;
		if (shaderProgram_ == 0) {
			throw std::runtime_error("Unable to load shape2D shaders!!");
		}
		indexMatViewport_ = glGetUniformLocation(shaderProgram_, "mViewportInverse");

		unsigned indexPos = glGetAttribLocation(shaderProgram_, "vPos");
		unsigned indexColor = glGetAttribLocation(shaderProgram_, "vColor");

		glBindVertexArray(lineVAO_);
		glBindBuffer(GL_ARRAY_BUFFER, lineVBO_);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lineIBO_);
		glEnableVertexAttribArray(indexPos);
		glEnableVertexAttribArray(indexColor);
		glVertexAttribPointer(indexPos, 3, GL_FLOAT, GL_FALSE, sizeof(s_lineVertex), (void*)offsetof(s_lineVertex, pos));
		glVertexAttribPointer(indexColor, 4, GL_FLOAT, GL_FALSE, sizeof(s_lineVertex), (void*)offsetof(s_lineVertex, rgba));

		glBindVertexArray(triangleVAO_);
		glBindBuffer(GL_ARRAY_BUFFER, triangleVBO_);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIBO_);
		glEnableVertexAttribArray(indexPos);
		glEnableVertexAttribArray(indexColor);
		glVertexAttribPointer(indexPos, 3, GL_FLOAT, GL_FALSE, sizeof(s_lineVertex), (void*)offsetof(s_lineVertex, pos));
		glVertexAttribPointer(indexColor, 4, GL_FLOAT, GL_FALSE, sizeof(s_lineVertex), (void*)offsetof(s_lineVertex, rgba));

		glBindVertexArray(0);
	});

	checkGLError("Shape2D:: vertex array creation");
}

Shape2D::~Shape2D() {
	glDeleteProgram(shaderProgram_);
	glDeleteVertexArrays(1, &triangleVAO_);
	glDeleteVertexArrays(1, &lineVAO_);
	glDeleteBuffers(1, &triangleVBO_);
	glDeleteBuffers(1, &triangleIBO_);
	glDeleteBuffers(1, &lineVBO_);
	glDeleteBuffers(1, &lineIBO_);
}

void Shape2D::unload() {
	delete instance;
	instance = nullptr;
}

void Shape2D::render(Viewport* vp, unsigned batchId) {
	PERF_MARKER_FUNC;
	assertDbg(batchId < batches_.size());

	glUseProgram(shaderProgram_);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendEquation(GL_BLEND_EQUATION_ALPHA);

	// set up viewport space settings:
	int vpw = vp->width(), vph = vp->height();
	float sx = 2.f / (vpw-1);
	float sy = -2.f / (vph-1);
	float sz = -1.e-2f;
	glm::mat4x4 matVP_to_UniformScale(glm::scale(glm::mat4(1), glm::vec3(sx, sy, sz)));
	glm::mat4x4 matVP_to_Uniform(glm::translate(matVP_to_UniformScale,
			glm::vec3(-vpw/2, -vph/2, 0)));
	glUniformMatrix4fv(indexMatViewport_, 1, GL_FALSE, glm::value_ptr(matVP_to_Uniform));

	auto &b = batches_[batchId];
	s_batch end {
		batchId < batches_.size() - 1 ? batches_[batchId+1].lineStripOffset_ : lineStrips_.size(),
		batchId < batches_.size() - 1 ? batches_[batchId+1].triangleOffset_ : triangleIndices_.size()
	};

	checkGLError("Shape2D::render() : setup");

	// render triangle primitives:
	glDisable(GL_CULL_FACE); // TODO do we need this?
	auto nTriIndices = end.triangleOffset_ - b.triangleOffset_;
	if (nTriIndices) {
		glBindVertexArray(triangleVAO_);
		glDrawElements(GL_TRIANGLES, nTriIndices, GL_UNSIGNED_SHORT, (void*)(sizeof(triangleIndices_[0]) * b.triangleOffset_));
		checkGLError("Shape2D::render() : glDrawElements #1");
	}

	// render line primitives
	glBindVertexArray(lineVAO_);
	for (unsigned l=b.lineStripOffset_; l < end.lineStripOffset_; l++) {
		glDrawElements(GL_LINES, lineStrips_[l].length, GL_UNSIGNED_SHORT, (void*)(sizeof(lineIndices_[0]) * lineStrips_[l].offset));
		checkGLError("Shape2D::render() : glDrawElements #2");
	}

	glBindVertexArray(0);
	glUseProgram(0);
	glDisable(GL_BLEND);
}

void Shape2D::startBatch() {
	auto lsoffs = batches_.empty() ? 0u : lineStrips_.size();
	auto trioffs = batches_.empty() ? 0u : triangleIndices_.size();
	batches_.push_back({lsoffs, trioffs});
}

void Shape2D::setupFrameData() {
	// populate device buffers
	glBindBuffer(GL_ARRAY_BUFFER, lineVBO_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(s_lineVertex) * lineBuffer_.size(), &lineBuffer_[0], GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lineIBO_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(lineIndices_[0]) * lineIndices_.size(), &lineIndices_[0], GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, triangleVBO_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(s_lineVertex) * triangleBuffer_.size(), &triangleBuffer_[0], GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIBO_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triangleIndices_[0]) * triangleIndices_.size(), &triangleIndices_[0], GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Shape2D::purgeRenderQueue() {
	lineBuffer_.clear();
	lineIndices_.clear();
	triangleBuffer_.clear();
	triangleIndices_.clear();
	lineStrips_.clear();
	batches_.clear();
}

void Shape2D::drawLine(glm::vec2 point1, glm::vec2 point2, float z, glm::vec3 rgb) {
	drawLine(point1, point2, z, glm::vec4(rgb, 1));
}

void Shape2D::drawLine(glm::vec2 point1, glm::vec2 point2, float z, glm::vec4 rgba) {
	PERF_MARKER_FUNC;
	lineStrips_.push_back({
		lineIndices_.size(),
		2
	});
	lineBuffer_.emplace_back(point1, z, rgba);
	lineIndices_.push_back(lineBuffer_.size()-1);
	lineBuffer_.emplace_back(point2, z, rgba);
	lineIndices_.push_back(lineBuffer_.size()-1);
}

void Shape2D::drawLineList(glm::vec2 verts[], int nVerts, float z, glm::vec3 rgb) {
	drawLineList(verts, nVerts, z, glm::vec4(rgb, 1));
}

void Shape2D::drawLineList(glm::vec2 verts[], int nVerts, float z, glm::vec4 rgba) {
	PERF_MARKER_FUNC;
	lineStrips_.push_back({
		lineIndices_.size(),
		nVerts
	});
	for (int i=0; i<nVerts; i++) {
		lineBuffer_.emplace_back(verts[i], z, rgba);
		lineIndices_.push_back(lineBuffer_.size()-1);
	}
}

void Shape2D::drawLineStrip(glm::vec2 verts[], int nVerts, float z, glm::vec3 rgb) {
	drawLineStrip(verts, nVerts, z, glm::vec4(rgb, 1));
}

void Shape2D::drawLineStrip(glm::vec2 verts[], int nVerts, float z, glm::vec4 rgba) {
	PERF_MARKER_FUNC;
	lineStrips_.push_back({
		lineIndices_.size(),
		(nVerts-1) * 2
	});
	for (int i=0; i<nVerts; i++) {
		lineBuffer_.emplace_back(verts[i], z, rgba);
		lineIndices_.push_back(lineBuffer_.size()-1);
		if (i > 0 && i < nVerts-1)
			lineIndices_.push_back(lineBuffer_.size()-1);
	}
}

void Shape2D::drawPolygon(glm::vec2 verts[], int nVerts, float z, glm::vec3 rgb) {
	drawPolygon(verts, nVerts, z, glm::vec4(rgb, 1));
}

void Shape2D::drawPolygon(glm::vec2 verts[], int nVerts, float z, glm::vec4 rgba) {
	PERF_MARKER_FUNC;
	lineStrips_.push_back({
		lineIndices_.size(),
		nVerts * 2
	});
	for (int i=0; i<nVerts; i++) {
		lineBuffer_.emplace_back(verts[i], z, rgba);
		lineIndices_.push_back(lineBuffer_.size()-1);
		if (i > 0) {
			lineIndices_.push_back(lineBuffer_.size()-1);
		}
	}
	lineIndices_.push_back(lineBuffer_.size()-nVerts);
}

void Shape2D::drawPolygonFilled(glm::vec2 verts[], int nVerts, float z, glm::vec3 rgb) {
	drawPolygonFilled(verts, nVerts, z, glm::vec4(rgb, 1));
}

void Shape2D::drawPolygonFilled(glm::vec2 verts[], int nVerts, float z, glm::vec4 rgba) {
	PERF_MARKER_FUNC;
	arrayContainer<glm::vec2> vtx(verts, nVerts);
	arrayContainer<decltype(vtx)> vtxWrap(&vtx, 1);
	std::vector<uint16_t> inds = mapbox::earcut<uint16_t>(vtxWrap);
	assertDbg(inds.size() % 3 == 0);

	triangleBuffer_.reserve(triangleBuffer_.size() + nVerts);
	unsigned base = triangleBuffer_.size();
	for (auto v = verts; v < verts+nVerts; v++)
		triangleBuffer_.emplace_back(*v, z, rgba);
	triangleIndices_.reserve(triangleIndices_.size() + inds.size());
	for (unsigned i=0; i<inds.size(); i++)
		triangleIndices_.push_back(base + inds[i]);
}

void Shape2D::drawRectangle(glm::vec2 pos, float z, glm::vec2 size, glm::vec3 rgb) {
	drawRectangle(pos, z, size, glm::vec4(rgb, 1));
}

void Shape2D::drawRectangle(glm::vec2 pos, float z, glm::vec2 size, glm::vec4 rgba) {
	PERF_MARKER_FUNC;
	glm::vec2 coords[] {
		pos,
		{pos.x, pos.y + size.y},
		pos + size,
		{pos.x + size.x, pos.y}
	};
	drawPolygon(coords, 4, z, rgba);
}

void Shape2D::drawRectangleCentered(glm::vec2 pos, float z, glm::vec2 size, glm::vec3 rgb) {
	drawRectangleCentered(pos, z, size, glm::vec4(rgb, 1));
}

void Shape2D::drawRectangleCentered(glm::vec2 pos, float z, glm::vec2 size, glm::vec4 rgba) {
	PERF_MARKER_FUNC;
	auto hSize = size * 0.5f;
	glm::vec2 coords[] {
		pos + glm::vec2{-hSize.x, -hSize.y},
		pos + glm::vec2{-hSize.x, +hSize.y},
		pos + glm::vec2{+hSize.x, +hSize.y},
		pos + glm::vec2{+hSize.x, -hSize.y}
	};
	drawPolygon(coords, 4, z, rgba);
}

void Shape2D::drawRectangleFilled(glm::vec2 pos, float z, glm::vec2 size, glm::vec3 rgb) {
	drawRectangleFilled(pos, z, size, glm::vec4(rgb, 1));
}

void Shape2D::drawRectangleFilled(glm::vec2 pos, float z, glm::vec2 size, glm::vec4 rgba) {
	glm::vec2 coords[] {
		pos,
		pos + glm::vec2{0, size.y},
		pos + size,
		pos + glm::vec2{size.x, 0}
	};
	drawPolygonFilled(coords, 4, z, rgba);
}

void makeCircle(glm::vec2 pos, float radius, int nSides, glm::vec2* outV) {
	PERF_MARKER_FUNC;
	// make a polygon out of the circle
	float phiStep = 2 * PI * 1.f / nSides;
	float phi = 0;
	for (int i=0; i<nSides; i++) {
		outV[i] = pos + glm::vec2{cosf(phi) * radius, sinf(phi) * radius};
		phi += phiStep;
	}
}

void Shape2D::drawCircle(glm::vec2 pos, float radius, float z, int nSides, glm::vec3 rgb) {
	drawCircle(pos, radius, z, nSides, glm::vec4(rgb, 1));
}

void Shape2D::drawCircle(glm::vec2 pos, float radius, float z, int nSides, glm::vec4 rgba) {
	PERF_MARKER_FUNC;
	glm::vec2 *v = new glm::vec2[nSides];
	makeCircle(pos, radius, nSides, v);
	drawPolygon(v, nSides, z, rgba);
	delete [] v;
}

void Shape2D::drawCircleFilled(glm::vec2 pos, float radius, float z, int nSides, glm::vec3 rgb) {
	drawCircleFilled(pos, radius, z, nSides, glm::vec4{rgb, 1});
}

void Shape2D::drawCircleFilled(glm::vec2 pos, float radius, float z, int nSides, glm::vec4 rgba) {
	PERF_MARKER_FUNC;
	glm::vec2 *v = new glm::vec2[nSides];
	makeCircle(pos, radius, nSides, v);
	drawPolygonFilled(v, nSides, z, rgba);
	delete [] v;
}
