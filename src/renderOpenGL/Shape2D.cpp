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
#include <boglfw/math/math3D.h>
#include <boglfw/utils/log.h>
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

Shape2D::Shape2D(Renderer* renderer)
	: lineShaderProgram_(0)
	, indexPos_(0)
	, indexColor_(0)
	, indexMatViewport_(0)
{
	renderer->registerRenderable(this);
	lineShaderProgram_ = Shaders::createProgram("data/shaders/shape2d.vert", "data/shaders/shape2d.frag");
	if (lineShaderProgram_ == 0) {
		throw std::runtime_error("Unable to load shape2D shaders!!");
	}
	indexPos_ = glGetAttribLocation(lineShaderProgram_, "vPos");
	indexColor_ = glGetAttribLocation(lineShaderProgram_, "vColor");
	indexMatViewport_ = glGetUniformLocation(lineShaderProgram_, "mViewportInverse");
}

Shape2D::~Shape2D() {
	glDeleteProgram(lineShaderProgram_);
}

void Shape2D::unload() {
	delete instance;
	instance = nullptr;
}

void Shape2D::render(Viewport* vp) {
	PERF_MARKER_FUNC;
	glUseProgram(lineShaderProgram_);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendEquation(GL_BLEND_EQUATION_ALPHA);

	glEnableVertexAttribArray(indexPos_);
	glEnableVertexAttribArray(indexColor_);

	// set up viewport space settings:
	int vpw = vp->width(), vph = vp->height();
	float sx = 2.f / (vpw-1);
	float sy = -2.f / (vph-1);
	float sz = -1.e-2f;
	glm::mat4x4 matVP_to_UniformScale(glm::scale(glm::mat4(1), glm::vec3(sx, sy, sz)));
	glm::mat4x4 matVP_to_Uniform(glm::translate(matVP_to_UniformScale,
			glm::vec3(-vpw/2, -vph/2, 0)));
	glUniformMatrix4fv(indexMatViewport_, 1, GL_FALSE, glm::value_ptr(matVP_to_Uniform));
	glEnableVertexAttribArray(indexPos_);
	glEnableVertexAttribArray(indexColor_);

	static std::vector<glm::vec3> posBuf;
	posBuf.reserve(max(buffer_.size(), max(bufferTri_.size(), buffer_.size())));
	static std::vector<glm::vec4> colorBuf;
	colorBuf.reserve(max(buffer_.size(), max(bufferTri_.size(), buffer_.size())));

	// build triangle vertex buffer:
	/*for (auto &v : bufferTri_) {
		posBuf.push_back(glm::vec3(v.pos.x(vp), v.pos.y(vp), v.z));
		colorBuf.push_back(v.rgba);
	}
	// render triangle primitives:
	glVertexAttribPointer(indexPos_, 3, GL_FLOAT, GL_FALSE, 0, &posBuf[0]);
	glVertexAttribPointer(indexColor_, 4, GL_FLOAT, GL_FALSE, 0, &colorBuf[0]);
	glDrawElements(GL_TRIANGLES, indicesTri_.size(), GL_UNSIGNED_SHORT, &indicesTri_[0]);*/

	// build buffers:
	posBuf.clear();
	colorBuf.clear();
	for (auto &v : buffer_) {
		posBuf.push_back(glm::vec3(v.pos.x(vp), v.pos.y(vp), v.z));
		colorBuf.push_back(v.rgba);
	}
	// do the actual drawing:
	glVertexAttribPointer(indexPos_, 3, GL_FLOAT, GL_FALSE, 0, &posBuf[0]);
	glVertexAttribPointer(indexColor_, 4, GL_FLOAT, GL_FALSE, 0, &colorBuf[0]);
	for (auto &b : batches_) {
		glDrawElements(GL_LINES, b.length, GL_UNSIGNED_SHORT, &indices_[b.offset]);
	}

	glDisable(GL_BLEND);
}

void Shape2D::purgeRenderQueue() {
	buffer_.clear();
	indices_.clear();
	bufferTri_.clear();
	indicesTri_.clear();
	batches_.clear();
}

void Shape2D::drawLine(glm::vec2 point1, glm::vec2 point2, float z, glm::vec3 rgb) {
	drawLine(point1, point2, z, glm::vec4(rgb, 1));
}

void Shape2D::drawLine(glm::vec2 point1, glm::vec2 point2, float z, glm::vec4 rgba) {
	PERF_MARKER_FUNC;
	batches_.push_back({
		indices_.size(),
		2
	});
	buffer_.push_back({point1, z, rgba});
	indices_.push_back(buffer_.size()-1);
	buffer_.push_back({point2, z, rgba});
	indices_.push_back(buffer_.size()-1);
}

void Shape2D::drawLineList(glm::vec2 verts[], int nVerts, float z, glm::vec3 rgb) {
	drawLineList(verts, nVerts, z, glm::vec4(rgb, 1));
}

void Shape2D::drawLineList(glm::vec2 verts[], int nVerts, float z, glm::vec4 rgba) {
	PERF_MARKER_FUNC;
	batches_.push_back({
		indices_.size(),
		nVerts
	});
	for (int i=0; i<nVerts; i++) {
		buffer_.push_back({verts[i], z, rgba});
		indices_.push_back(buffer_.size()-1);
	}
}

void Shape2D::drawLineStrip(glm::vec2 verts[], int nVerts, float z, glm::vec3 rgb) {
	drawLineStrip(verts, nVerts, z, glm::vec4(rgb, 1));
}

void Shape2D::drawLineStrip(glm::vec2 verts[], int nVerts, float z, glm::vec4 rgba) {
	PERF_MARKER_FUNC;
	batches_.push_back({
		indices_.size(),
		(nVerts-1) * 2
	});
	for (int i=0; i<nVerts; i++) {
		buffer_.push_back({verts[i], z, rgba});
		indices_.push_back(buffer_.size()-1);
		if (i > 0 && i < nVerts-1)
			indices_.push_back(buffer_.size()-1);
	}
}

void Shape2D::drawPolygon(glm::vec2 verts[], int nVerts, float z, glm::vec3 rgb) {
	drawPolygon(verts, nVerts, z, glm::vec4(rgb, 1));
}

void Shape2D::drawPolygon(glm::vec2 verts[], int nVerts, float z, glm::vec4 rgba) {
	PERF_MARKER_FUNC;
	batches_.push_back({
		indices_.size(),
		nVerts * 2
	});
	for (int i=0; i<nVerts; i++) {
		buffer_.push_back({verts[i], z, rgba});
		indices_.push_back(buffer_.size()-1);
		if (i > 0) {
			indices_.push_back(buffer_.size()-1);
		}
	}
	indices_.push_back(buffer_.size()-nVerts);
}

void Shape2D::drawPolygonFilled(glm::vec2 verts[], int nVerts, float z, glm::vec3 rgb) {
	drawPolygonFilled(verts, nVerts, z, glm::vec4(rgb, 1));
}

void Shape2D::drawPolygonFilled(glm::vec2 verts[], int nVerts, float z, glm::vec4 rgba) {
	//TODO must tesselate into triangles
}

void Shape2D::drawRectangle(glm::vec2 pos, float z, glm::vec2 size, glm::vec3 rgb) {
	drawRectangle(pos, z, size, glm::vec4(rgb, 1));
}

void Shape2D::drawRectangle(glm::vec2 pos, float z, glm::vec2 size, glm::vec4 rgba) {
	PERF_MARKER_FUNC;
	glm::vec2 coords[] {
		pos,
		pos + size.y(),
		pos + size,
		pos + size.x()
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
		pos - hSize,
		pos - hSize + hSize.y(),
		pos + hSize,
		pos + hSize - hSize.y()
	};
	drawPolygon(coords, 4, z, rgba);
}

void Shape2D::drawRectangleFilled(glm::vec2 pos, float z, glm::vec2 size, glm::vec3 rgb) {
	drawRectangleFilled(pos, z, size, glm::vec4(rgb, 1));
}

void Shape2D::drawRectangleFilled(glm::vec2 pos, float z, glm::vec2 size, glm::vec4 rgba) {
	glm::vec2 coords[] {
		pos,
		pos + size.y(),
		pos + size,
		pos + size.x()
	};
	drawPolygonFilled(coords, 4, z, rgba);
}

void Shape2D::drawCircle(glm::vec2 pos, float radius, float z, int nSides, glm::vec3 rgb) {
	drawCircle(pos, radius, z, nSides, glm::vec4(rgb, 1));
}

void Shape2D::drawCircle(glm::vec2 pos, float radius, float z, int nSides, glm::vec4 rgba) {
	PERF_MARKER_FUNC;
	// make a polygon out of the circle
	float phiStep = 2 * PI * 1.f / nSides;
	glm::vec2 *v = new glm::vec2[nSides];
	float phi = 0;
	for (int i=0; i<nSides; i++) {
		v[i] = pos + glm::vec2{cosf(phi) * radius, sinf(phi) * radius};
		phi += phiStep;
	}
	drawPolygon(v, nSides, z, rgba);
	delete [] v;
}
