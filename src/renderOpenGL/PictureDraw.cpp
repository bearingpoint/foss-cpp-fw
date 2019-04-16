#include <boglfw/renderOpenGL/PictureDraw.h>
#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/Camera.h>
#include <boglfw/renderOpenGL/shader.h>
#include <boglfw/renderOpenGL/glToolkit.h>
#include <boglfw/utils/log.h>

#include <glm/gtc/type_ptr.hpp>

static PictureDraw* instance = nullptr;

struct PictureDraw::RenderData {
	unsigned VAO = 0;
	unsigned VBO = 0;
	unsigned IBO = 0;
	unsigned program = 0;

	int iPos;
	int iUV;
	int iuColor;
	int iuBlendFactor;
	int iuBlendMode;
	int iuMulTexAlpha;
	int iuMatV2U;

	RenderData() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &IBO);
	}

	~RenderData() {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &IBO);
	}
};

struct PictureVertex {
	glm::vec2 pos;
	glm::vec2 uv;
};

PictureDraw* PictureDraw::get() {
	assertDbg(instance && "must be initialized first - call RenderHelpers::load()!");
	return instance;
}

void PictureDraw::init() {
	instance = new PictureDraw();
}

void PictureDraw::unload() {
	delete instance, instance = nullptr;
}

PictureDraw::PictureDraw() {
	LOGPREFIX("PictureDraw");
	pRenderData = new RenderData();
	Shaders::createProgram("data/shaders/picture.vert", "data/shaders/picture.frag", [this](unsigned id) {
		pRenderData->program = id;
		if (pRenderData->program == 0) {
			ERROR("Unable to load picture shaders!!");
			return;
		}
		pRenderData->iuMatV2U = glGetUniformLocation(pRenderData->program, "mV2U");

		pRenderData->iPos = glGetAttribLocation(pRenderData->program, "pos");
		pRenderData->iUV = glGetAttribLocation(pRenderData->program, "uv");
		pRenderData->iuBlendFactor = glGetUniformLocation(pRenderData->program, "blendFactor");
		pRenderData->iuBlendMode = glGetUniformLocation(pRenderData->program, "blendMode");
		pRenderData->iuColor = glGetUniformLocation(pRenderData->program, "blendColor");
		pRenderData->iuMatV2U = glGetUniformLocation(pRenderData->program, "matV2U");
		pRenderData->iuMulTexAlpha = glGetUniformLocation(pRenderData->program, "mulTextureAlpha");

		glBindVertexArray(pRenderData->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, pRenderData->VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pRenderData->IBO);
		glEnableVertexAttribArray(pRenderData->iPos);
		glEnableVertexAttribArray(pRenderData->iUV);
		glVertexAttribPointer(pRenderData->iPos, 3, GL_FLOAT, GL_FALSE, sizeof(PictureVertex), (void*)offsetof(PictureVertex, pos));
		glVertexAttribPointer(pRenderData->iUV, 2, GL_FLOAT, GL_FALSE, sizeof(PictureVertex), (void*)offsetof(PictureVertex, uv));

		glBindVertexArray(0);
	});

	checkGLError("Shape2D:: vertex array creation");
}

PictureDraw::~PictureDraw() {
	delete pRenderData, pRenderData = nullptr;
}

void PictureDraw::flush() {

}

// draws a texture in viewport coordinates
void PictureDraw::draw(int texId, glm::vec2 pos, glm::vec2 size) {

}

// draws a texture with advanced blending
void PictureDraw::draw(int texId, glm::vec2 pos, glm::vec2 size, BlendOperation blendOp) {

}
