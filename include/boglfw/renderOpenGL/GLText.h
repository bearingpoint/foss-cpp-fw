/*
 * GLText.h
 *
 *  Created on: Nov 22, 2014
 *      Author: bog
 */

#ifndef RENDEROPENGL_GLTEXT_H_
#define RENDEROPENGL_GLTEXT_H_

#include <string>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

class GLText {
public:
	static void disableMipMaps(bool disable) { disableMipMaps_ = disable; }
	static GLText* get();
	virtual ~GLText();

	// flush - all pending print commands will be executed and all following commands
	// will produce text that will be layered on top of everything previous.
	// This is useful when interleaving draw calls with other 2D drawing code to achieve layering (such as with Shape2D)
	void flush();

	// prints text - [pos] is the bottom-left corner of the text
	void print(const std::string &text, glm::vec2 pos, int size, glm::vec3 const& color);
	void print(const std::string &text, glm::vec2 pos, int size, glm::vec4 const& color);

	glm::vec2 getTextRect(std::string const& text, int fontSize);

protected:
	friend class RenderHelpers;
	static void init(const char* fontPath); // path to font *.desc file
	static void unload();
private:
	GLText(const char * texturePath, int rows, int cols, char firstChar, int defaultSize);

	unsigned textureID_;			// Texture containing the font
	unsigned VAO_;
	unsigned posVBO_;				// Buffer containing the vertices
	unsigned uvVBO_;				// UVs
	unsigned colorVBO_;				// vertex colors
	unsigned shaderProgram_;		// Program used to render the text
	unsigned indexViewportHalfSize_;
	unsigned indexTranslation_;
	unsigned u_textureID_;			// Location of the program's texture attribute
	int rows_, cols_, firstChar_;
	float cellRatio_; 					// cellWeight / cellHidth
	int defaultSize_;					// text size from the texture
	std::vector<glm::vec2> vertices_;	// these are relative to item's position (below)
	std::vector<glm::vec2> UVs_;
	std::vector<glm::vec4> colors_;
	std::vector<glm::vec2> itemPositions_;
	std::vector<int> verticesPerItem_;

	static bool disableMipMaps_;
};

#endif /* RENDEROPENGL_GLTEXT_H_ */
