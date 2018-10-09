/*
 * Shape2D.h
 *
 *  Created on: Nov 14, 2014
 *      Author: bogdan
 */

#ifndef RENDEROPENGL_SHAPE2D_H_
#define RENDEROPENGL_SHAPE2D_H_

#include "IRenderable.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <vector>

class Renderer;
class Viewport;

// renders 2D shapes in viewport space
class Shape2D : public IRenderable {
public:
	static Shape2D* get();
	virtual ~Shape2D() override;
	static void init(Renderer* renderer);

	// draw a single line segment
	void drawLine(glm::vec2 point1, glm::vec2 point2, float z, glm::vec3 rgb);
	void drawLine(glm::vec2 point1, glm::vec2 point2, float z, glm::vec4 rgba);
	// draw a list of separate lines (pairs of two vertices)
	void drawLineList(glm::vec2 verts[], int nVerts, float z, glm::vec3 rgb);
	void drawLineList(glm::vec2 verts[], int nVerts, float z, glm::vec4 rgba);
	// draw a line strip (connected lines)
	void drawLineStrip(glm::vec2 verts[], int nVerts, float z, glm::vec3 rgb);
	void drawLineStrip(glm::vec2 verts[], int nVerts, float z, glm::vec4 rgba);
	// draw a rectangle; pos is the top-left position
	void drawRectangle(glm::vec2 pos, float z, glm::vec2 size, glm::vec3 rgb);
	void drawRectangle(glm::vec2 pos, float z, glm::vec2 size, glm::vec4 rgba);
	// draw a rectangle; pos is the center position
	void drawRectangleCentered(glm::vec2 pos, float z, glm::vec2 size, glm::vec3 rgb);
	void drawRectangleCentered(glm::vec2 pos, float z, glm::vec2 size, glm::vec4 rgba);
	// draw a filled rectangle; pos is the center position
	void drawRectangleFilled(glm::vec2 pos, float z, glm::vec2 size, glm::vec3 rgb);
	void drawRectangleFilled(glm::vec2 pos, float z, glm::vec2 size, glm::vec4 rgba);
	// draw a polygon
	void drawPolygon(glm::vec2 verts[], int nVerts, float z, glm::vec3 rgb);
	void drawPolygon(glm::vec2 verts[], int nVerts, float z, glm::vec4 rgba);
	// draw a filled polygon
	void drawPolygonFilled(glm::vec2 verts[], int nVerts, float z, glm::vec3 rgb);
	void drawPolygonFilled(glm::vec2 verts[], int nVerts, float z, glm::vec4 rgba);
	// draw a circle
	void drawCircle(glm::vec2 pos, float radius, float , int nSides, glm::vec3 rgb);
	void drawCircle(glm::vec2 pos, float radius, float , int nSides, glm::vec4 rgba);
	// draw a filled circle
	void drawCircleFilled(glm::vec2 pos, float radius, float , int nSides, glm::vec3 rgb);
	void drawCircleFilled(glm::vec2 pos, float radius, float , int nSides, glm::vec4 rgba);

protected:
	Shape2D(Renderer* renderer);

private:
	void render(Viewport* vp) override;
	void purgeRenderQueue() override;
	void unload() override;

	struct s_lineVertex {
		glm::vec2 pos;
		float z;
		glm::vec4 rgba; 	// color

		s_lineVertex(glm::vec2 pos, float z, glm::vec4 rgba)
			: pos(pos), z(z), rgba(rgba) {}
	};
	// line buffers
	std::vector<s_lineVertex> buffer_;
	std::vector<unsigned short> indices_;

	struct s_batch {
		size_t offset;	// offset in index buffer
		size_t length;	// number of indices
	};
	std::vector<s_batch> batches_;

	// triangle buffers
	std::vector<s_lineVertex> bufferTri_;
	std::vector<unsigned short> indicesTri_;

	unsigned lineShaderProgram_;
	unsigned indexPos_;
	unsigned indexColor_;
	unsigned indexMatViewport_;
};

#endif /* RENDEROPENGL_SHAPE2D_H_ */
