/*
 * text.cpp
 *
 *  Created on: Nov 22, 2014
 *      Author: bog
 */

#include <boglfw/renderOpenGL/GLText.h>
#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/TextureLoader.h>
#include <boglfw/renderOpenGL/shader.h>
#include <boglfw/renderOpenGL/ViewportCoord.h>
#include <boglfw/renderOpenGL/RenderHelpers.h>
#include <boglfw/utils/configFile.h>
#include <boglfw/utils/assert.h>
#include <boglfw/utils/log.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>

#include <vector>
#include <map>

using namespace glm;

static GLText* instance = nullptr;
bool GLText::disableMipMaps_ = false;

void GLText::init(const char* fontPath) {
	// load font file and parse properties
	const std::string kTexture = "texture";
	const std::string kRows = "rows";
	const std::string kColumns = "columns";
	const std::string kFirstChar = "firstChar";
	const std::string kDefaultSize = "defaultSize";
	std::map<std::string, std::string> opts {
		{ kTexture, {}},
		{ kRows, {}},
		{ kColumns, {}},
		{ kFirstChar, {}},
		{ kDefaultSize, {}}
	};
	std::vector<std::string> req { kTexture, kRows, kColumns, kFirstChar, kDefaultSize };
	if (!parseConfigFile(fontPath, opts, req)) {
		ERROR("Loading font from " << fontPath);
		return;
	}
	if (opts[kFirstChar].size() != 3) {
		ERROR("Parsing 'firstChar' value from font desc file. Must be in the form: 'c' (including quotes).");
		return;
	}
	auto texturePath = opts[kTexture];
	auto rows = atoi(opts[kRows].c_str());
	auto cols = atoi(opts[kColumns].c_str());
	auto defaultSize = atoi(opts[kDefaultSize].c_str());
	char firstChar = opts[kFirstChar][1];
	instance = new GLText(texturePath.c_str(), rows, cols, firstChar, defaultSize);
}

GLText* GLText::get() {
	assertDbg(instance && "must be initialized first - call RenderHelpers::load()!");
	return instance;
}

void GLText::unload() {
	delete instance, instance = nullptr;
}

GLText::GLText(const char * texturePath, int rows, int cols, char firstChar, int defaultSize)
	: rows_(rows), cols_(cols), firstChar_(firstChar), defaultSize_(defaultSize)
{
	cellRatio_ = (float)rows/cols;
	// Initialize texture
	textureID_ = TextureLoader::loadFromPNG(texturePath, true);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID_);
	if (!disableMipMaps_) {
		glGenerateMipmap(GL_TEXTURE_2D);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Initialize VAO & VBOs
	glGenVertexArrays(1, &VAO_);
	glGenBuffers(1, &posVBO_);
	glGenBuffers(1, &uvVBO_);
	glGenBuffers(1, &colorVBO_);

	// Initialize Shader
	Shaders::createProgram("data/shaders/text.vert", "data/shaders/text.frag", [this] (unsigned id) {
		shaderProgram_ = id;
		if (!shaderProgram_) {
			ERROR("Could not load text shaders!");
			return;
		}
		// Get a handle for our buffers
		unsigned vertexPosition_screenspaceID_ = glGetAttribLocation(shaderProgram_, "vertexPosition_screenspace");
		unsigned vertexUVID_ = glGetAttribLocation(shaderProgram_, "vertexUV");
		unsigned vertexColorID_ = glGetAttribLocation(shaderProgram_, "vertexColor");

		// Initialize uniforms' IDs
		indexViewportHalfSize_ = glGetUniformLocation(shaderProgram_, "viewportHalfSize");
		indexTranslation_ = glGetUniformLocation(shaderProgram_, "translation");
		u_textureID_ = glGetUniformLocation( shaderProgram_, "myTextureSampler" );

		glBindVertexArray(VAO_);

		glBindBuffer(GL_ARRAY_BUFFER, posVBO_);
		glEnableVertexAttribArray(vertexPosition_screenspaceID_);
		glVertexAttribPointer(vertexPosition_screenspaceID_, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, uvVBO_);
		glEnableVertexAttribArray(vertexUVID_);
		glVertexAttribPointer(vertexUVID_, 2, GL_FLOAT, GL_FALSE, 0, 0);

		// 3rd attribute buffer : vertex colors
		glBindBuffer(GL_ARRAY_BUFFER, colorVBO_);
		glEnableVertexAttribArray(vertexColorID_);
		glVertexAttribPointer(vertexColorID_, 4, GL_FLOAT, GL_FALSE, 0, 0);

		glBindVertexArray(0);
	});
}

GLText::~GLText() {
	// Delete buffers
	glDeleteBuffers(1, &posVBO_);
	glDeleteBuffers(1, &uvVBO_);
	glDeleteBuffers(1, &colorVBO_);

	glDeleteVertexArrays(1, &VAO_);

	// Delete texture
	glDeleteTextures(1, &textureID_);

	// Delete shader
	glDeleteProgram(shaderProgram_);
}

glm::vec2 GLText::getTextRect(const std::string& text, int fontSize) {
	unsigned int length = text.length();
	float xSize = fontSize*cellRatio_;
	int x = 0;
	int y = 0;
	int lineX = 0;
	int lineH = fontSize * 0.75f;
	for ( unsigned int i=0 ; i<length ; i++ ) {
		char character = text[i];
		if (character == '\t') {
			lineX += 4 * xSize;
			continue;
		}
		if (character == '\n') {
			y += lineH;
			if (lineX > x)
				x = lineX;
			lineX = 0;
			continue;
		}
		lineX += xSize;
	}
	if (lineX > x)
		x = lineX;
	return glm::vec2(x, y+lineH);
}

void GLText::print(const std::string &text, ViewportCoord pos, int size, glm::vec3 const& color) {
	print(text, pos, size, glm::vec4(color, 1));
}

void GLText::print(const std::string &text, ViewportCoord pos, int size, glm::vec4 const& color) {
	unsigned int length = text.length();
	float xSize = size*cellRatio_;
	glm::vec4 altColor = color;
	if (size < defaultSize_)
		altColor.a *= (float)defaultSize_ / size;

	// Fill buffers
	itemPositions_.push_back(pos);
	size_t nPrevVertices = vertices_.size();
	int x = 0;
	int y = 0;
	int initialX = 0;
	for ( unsigned int i=0 ; i<length ; i++ ) {
		char character = text[i];
		if (character == '\t') {
			x += 4 * xSize;
			continue;
		}
		if (character == '\n') {
			y += size * 0.75f;
			x = initialX;
			continue;
		}

		glm::vec2 vertex_up_left    { x      , y-size };
		glm::vec2 vertex_up_right   { x+xSize, y-size };
		glm::vec2 vertex_down_right { x+xSize, y };
		glm::vec2 vertex_down_left  { x      , y };

		x += xSize;

		vertices_.push_back(vertex_up_left   );
		vertices_.push_back(vertex_down_left );
		vertices_.push_back(vertex_up_right  );

		vertices_.push_back(vertex_down_right);
		vertices_.push_back(vertex_up_right);
		vertices_.push_back(vertex_down_left);

		float uv_x = ((character - firstChar_) % cols_) / (float)cols_;
		float uv_y = 1.f - ((character - firstChar_) / cols_) / (float)rows_;

		glm::vec2 uv_up_left    = glm::vec2( uv_x          , uv_y );
		glm::vec2 uv_up_right   = glm::vec2( uv_x+1.0f/cols_, uv_y );
		glm::vec2 uv_down_right = glm::vec2( uv_x+1.0f/cols_, uv_y - 1.0f/rows_ );
		glm::vec2 uv_down_left  = glm::vec2( uv_x          , uv_y - 1.0f/rows_ );
		UVs_.push_back(uv_up_left   );
		UVs_.push_back(uv_down_left );
		UVs_.push_back(uv_up_right  );

		UVs_.push_back(uv_down_right);
		UVs_.push_back(uv_up_right);
		UVs_.push_back(uv_down_left);

		for (int ci=0; ci<6; ci++)
			colors_.push_back(altColor);
	}

	verticesPerItem_.push_back(vertices_.size() - nPrevVertices);
}

void GLText::flush() {
	if (!shaderProgram_)
		return;

	Viewport* pCrtViewport = RenderHelpers::getActiveViewport();
	if (!pCrtViewport) {
		assertDbg(!!!"No viewport is currently rendering!");
		return;
	}
	Viewport const& vp = *pCrtViewport;

	unsigned nItems = itemPositions_.size();
	if (!nItems)
		return;

	// update render buffers:
	glBindBuffer(GL_ARRAY_BUFFER, posVBO_);
	glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(vertices_[0]), &vertices_[0], GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, uvVBO_);
	glBufferData(GL_ARRAY_BUFFER, UVs_.size() * sizeof(UVs_[0]), &UVs_[0], GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, colorVBO_);
	glBufferData(GL_ARRAY_BUFFER, colors_.size() * sizeof(colors_[0]), &colors_[0], GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Bind shader
	glUseProgram(shaderProgram_);

	// Bind texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID_);
	// Set our "myTextureSampler" sampler to user Texture Unit 0
	glUniform1i(u_textureID_, 0);

	vec2 halfVP(vp.width() / 2, vp.height() / 2);
	glUniform2fv(indexViewportHalfSize_, 1, &halfVP[0]);

	glBindVertexArray(VAO_);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	// do the drawing:
	unsigned offset = 0;
	for (unsigned i=0; i<nItems; i++) {
		glm::vec2 translate(itemPositions_[i].x(vp), itemPositions_[i].y(vp));
		glUniform2fv(indexTranslation_, 1, &translate[0]);
		glDrawArrays(GL_TRIANGLES, offset, verticesPerItem_[i]);
		offset += verticesPerItem_[i];
	}

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glBindVertexArray(0);
	glUseProgram(0);

	// purge all cached data:
	vertices_.clear();
	UVs_.clear();
	colors_.clear();
	itemPositions_.clear();
	verticesPerItem_.clear();
}

