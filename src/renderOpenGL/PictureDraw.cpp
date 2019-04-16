#include <boglfw/renderOpenGL/PictureDraw.h>
#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/Camera.h>
#include <boglfw/renderOpenGL/shader.h>
#include <boglfw/renderOpenGL/glToolkit.h>
#include <boglfw/renderOpenGL/RenderHelpers.h>
#include <boglfw/utils/log.h>
#include <boglfw/perf/marker.h>

#include <glm/gtc/type_ptr.hpp>

static PictureDraw* instance = nullptr;

struct PictureDraw::RenderData {
	unsigned VAO = 0;
	unsigned VBO = 0;
	//unsigned IBO = 0;
	unsigned program = 0;

	int iPos;
	int iUV;
	int iuColor;
	int iuBlendFactor;
	int iuBlendMode;
	int iuMulTexAlpha;
	int iuMatV2U;
	int iuTexture;

	RenderData() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		//glGenBuffers(1, &IBO);
	}

	~RenderData() {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		//glDeleteBuffers(1, &IBO);
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
		pRenderData->iuTexture = glGetUniformLocation(pRenderData->program, "texPicture");

		glBindVertexArray(pRenderData->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, pRenderData->VBO);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pRenderData->IBO);
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
	PERF_MARKER_FUNC;

	// populate device buffers
	glBindBuffer(GL_ARRAY_BUFFER, pRenderData->VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(PictureVertex) * verts_.size(), &verts_[0], GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUseProgram(pRenderData->program);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	// set up viewport space settings:
	Viewport* vp = RenderHelpers::getActiveViewport();
	if (!vp) {
		assertDbg(!!!"No viewport is currently rendering!");
		return;
	}
	glm::mat4x4 matVP_to_Uniform = vp->viewport2Uniform();
	glUniformMatrix4fv(pRenderData->iuMatV2U, 1, GL_FALSE, glm::value_ptr(matVP_to_Uniform));
	glActiveTexture(GL_TEXTURE0);

	checkGLError("PictureDraw::flush() : setup");

	glBindVertexArray(pRenderData->VAO);
	for (unsigned i=0; i<blendOps_.size(); i++) {
		glUniform1f(pRenderData->iuBlendFactor, blendOps_[i].blendFactor);
		glUniform1i(pRenderData->iuBlendMode, (int)blendOps_[i].blendMode);
		glUniform4fv(pRenderData->iuColor, 1, glm::value_ptr(blendOps_[i].blendColor));
		glUniform1i(pRenderData->iuMulTexAlpha, blendOps_[i].multiplyByTextureAlpha ? 1 : 0);

		glBindTexture(GL_TEXTURE_2D, texIds_[i]);
		glUniform1i(pRenderData->iuTexture, 0);

		unsigned vOffs = i * 4;
		glDrawArrays(GL_TRIANGLE_STRIP, vOffs, 4);
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glUseProgram(0);

	checkGLError("PictureDraw::flush() : done");

	blendOps_.clear();
	verts_.clear();
}

// draws a texture in viewport coordinates
void PictureDraw::draw(int texId, glm::vec2 pos, glm::vec2 size) {
	BlendOperation blendOp {
		BlendOperation::MODE_NORMAL,	// blend mode
		{ 1.f, 0.f, 0.f, 1.f },			// blend color
		1.f,							// blend factor
		false,							// multiply texture alpha
	};
	draw(texId, pos, size, blendOp);
}

// draws a texture with advanced blending
void PictureDraw::draw(int texId, glm::vec2 pos, glm::vec2 size, BlendOperation blendOp) {

	PictureVertex top_left {
		pos,
		{0.f, 1.f}
	};
	PictureVertex top_right {
		pos + glm::vec2{size.x, 0.f},
		{1.f, 1.f}
	};
	PictureVertex bottom_left {
		pos + glm::vec2{0.f, size.y},
		{0.f, 0.f}
	};
	PictureVertex bottom_right {
		pos + size,
		{1.f, 0.f}
	};

	verts_.push_back(top_left);
	verts_.push_back(top_right);
	verts_.push_back(bottom_left);
	verts_.push_back(bottom_right);

	blendOps_.push_back(blendOp);
	texIds_.push_back(texId);
}
