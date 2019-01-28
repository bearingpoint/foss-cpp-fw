/*
 * Mesh.cpp
 *
 *  Created on: Apr 24, 2017
 *      Author: bog
 */

#include <boglfw/renderOpenGL/Mesh.h>

#include <GL/glew.h>

Mesh::Mesh() {
	unsigned bufs[2];
	glGenBuffers(2, bufs);
	VBO_ = bufs[0];
	IBO_ = bufs[1];
	glGenVertexArrays(1, &VAO_);
}

Mesh::~Mesh() {
	glDeleteVertexArrays(1, &VAO_);
	unsigned bufs[] { VBO_, IBO_ };
	glDeleteBuffers(2, bufs);
}

void Mesh::createBox(glm::vec3 center, float width, float height, float depth) {
	float left = center.x - width * 0.5f;
	float right = center.x + width * 0.5f;
	float bottom = center.y - height * 0.5f;
	float top = center.y + height * 0.5f;
	float back = center.z - depth * 0.5f;
	float front = center.z + depth * 0.5f;
	glm::vec3 nBack(0, 0, -1);
	glm::vec3 nFront(0, 0, 1);
	glm::vec3 nLeft(-1, 0, 0);
	glm::vec3 nRight(1, 0, 0);
	glm::vec3 nTop(0, 1, 0);
	glm::vec3 nBottom(0, -1, 0);
	glm::vec4 white(1, 1, 1, 1);
	s_Vertex vertices[] {
	// back face
		// #0 back bottom left
		{
			{left, bottom, back}, nBack, {0, 0}, white
		},
		// #1 back top left
		{
			{left, top, back}, nBack, {0, 1}, white
		},
		// #2 back top right
		{
			{right, top, back}, nBack, {1, 1}, white
		},
		// #3 back bottom right
		{
			{right, bottom, back}, nBack, {1, 0}, white
		},
	// top face
		// #4 back top left
		{
			{left, top, back}, nTop, {0, 0}, white
		},
		// #5 front top left
		{
			{left, top, front}, nTop, {0, 1}, white
		},
		// #6 front top right
		{
			{right, top, front}, nTop, {1, 1}, white
		},
		// #7 back top right
		{
			{right, top, back}, nTop, {1, 0}, white
		},
	// front face
		// #8 front top right
		{
			{right, top, front}, nFront, {0, 0}, white
		},
		// #9 front top left
		{
			{left, top, front}, nFront, {0, 1}, white
		},
		// #10 front bottom left
		{
			{left, bottom, front}, nFront, {1, 1}, white
		},
		// #11 front bottom right
		{
			{right, bottom, front}, nFront, {1, 0}, white
		},
	// bottom face
		// #12 front bottom left
		{
			{left, bottom, front}, nBottom, {0, 0}, white
		},
		// #13 back bottom left
		{
			{left, bottom, back}, nBottom, {0, 1}, white
		},
		// #14 back bottom right
		{
			{right, bottom, back}, nBottom, {1, 1}, white
		},
		// #15 front bottom right
		{
			{right, bottom, front}, nBottom, {1, 0}, white
		},
	// left face
		// #16 front bottom left
		{
			{left, bottom, front}, nLeft, {0, 0}, white
		},
		// #17 front top left
		{
			{left, top, front}, nLeft, {0, 1}, white
		},
		// #18 back top left
		{
			{left, top, back}, nLeft, {1, 1}, white
		},
		// #19 back bottom left
		{
			{left, bottom, back}, nLeft, {1, 0}, white
		},
	// right face
		// #20 back bottom right
		{
			{right, bottom, front}, nRight, {0, 0}, white
		},
		// #21 back top right
		{
			{right, top, front}, nRight, {0, 1}, white
		},
		// #22 front top right
		{
			{right, top, back}, nRight, {1, 1}, white
		},
		// #23 front bottom right
		{
			{right, bottom, back}, nRight, {1, 0}, white
		}
	};

#if (1)	// debug vertices with colors
	glm::vec4 c[] { {1, 0, 0, 1}, {0, 1, 0, 1}, {0, 0, 1, 1}, {1, 1, 0, 1} };
	for (unsigned i=0; i<sizeof(vertices) / sizeof(vertices[0]); i++) {
		vertices[i].color = c[i % (sizeof(c) / sizeof(c[0]))];
	}
#endif

	glBindBuffer(GL_ARRAY_BUFFER, VBO_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	uint16_t indices[] {
		// back face
		0, 1, 2, 0, 2, 3,
		// top face
		4, 5, 6, 4, 6, 7,
		// front face
		8, 9, 10, 8, 10, 11,
		// bottom face
		12, 13, 14, 12, 14, 15,
		// left face
		16, 17, 18, 16, 18, 19,
		// right face
		20, 21, 22, 20, 22, 23
	};
	indexCount_ = sizeof(indices) / sizeof(indices[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	mode_ = RENDER_MODE_TRIANGLES;
}

void Mesh::createSphere(glm::vec3 center, float radius, int detail) {

}

void Mesh::createGizmo(float axisLength) {
	float axl = axisLength;
	s_Vertex verts[] {
		// X axis
		{ {0.f, 0.f, 0.f}, {0.f, 0.f, -1.f}, {0.f, 0.f}, {1.0f, 0.8f, 0.8f, 1.f} },
		{ {axl, 0.f, 0.f}, {0.f, 0.f, -1.f}, {1.f, 0.f}, {0.5f, 0.f, 0.f, 1.f} },
		// Y axis
		{ {0.f, 0.f, 0.f}, {0.f, 0.f, -1.f}, {0.f, 0.f}, {0.8f, 1.f, 0.8f, 1.f} },
		{ {0.f, axl, 0.f}, {0.f, 0.f, -1.f}, {1.f, 0.f}, {0.f, 0.5f, 0.f, 1.f} },
		// Z axis
		{ {0.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f}, {0.8f, 0.8f, 1.f, 1.f} },
		{ {0.f, 0.f, axl}, {0.f, 1.f, 0.f}, {1.f, 0.f}, {0.f, 0.f, 0.5f, 1.f} },
	};
	glBindBuffer(GL_ARRAY_BUFFER, VBO_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	uint16_t inds[] {
		0, 1, 2, 3, 4, 5
	};
	indexCount_ = sizeof(inds) / sizeof(inds[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(inds), inds, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	mode_ = RENDER_MODE_LINES;
}
