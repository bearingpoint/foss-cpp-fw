/*
 * Mesh.h
 *
 *  Created on: Apr 24, 2017
 *      Author: bog
 */

#ifndef RENDEROPENGL_MESH_H_
#define RENDEROPENGL_MESH_H_

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>

class Mesh {
public:

	enum RENDER_MODES {
		RENDER_MODE_POINTS,
		RENDER_MODE_LINES,
		RENDER_MODE_TRIANGLES,
		RENDER_MODE_TRIANGLES_WIREFRAME
	};

	Mesh();
	virtual ~Mesh();

	void createBox(glm::vec3 center, float width, float height, float depth);
	void createSphere(glm::vec3 center, float radius, int detail=10);
	void createGizmo(float axisLength);
	
	void setRenderMode(RENDER_MODES mode) { mode_ = mode; }

	unsigned getVAO() const { return VAO_; }
	//unsigned getIBO() const { return IBO_; }
	unsigned getRenderMode() const;
	unsigned getElementsCount() const { return indexCount_; }

private:
	friend class MeshRenderer;

	unsigned VAO_ = 0;
	unsigned VBO_ = 0;
	unsigned IBO_ = 0;
	unsigned indexCount_ = 0;
	bool vertexAttribsSet_ = false;
	RENDER_MODES mode_ = RENDER_MODE_TRIANGLES;

	struct s_Vertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 UV1;
		glm::vec3 color;
	};
};

#endif /* RENDEROPENGL_MESH_H_ */
