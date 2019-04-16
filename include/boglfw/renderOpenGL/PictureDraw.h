#ifndef BOGLFW_PICTURE_DRAW_H
#define BOGLFW_PICTURE_DRAW_H

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include <vector>

struct BlendOperation {
	enum BlendMode {
		MODE_NORMAL = 0,		// linear interpolation between blendColor and texture, according to blendFactor
		MODE_ADDITIVE = 1,		// blendColor + blendFactor * texture
		MODE_MULTIPLY = 2,		// blendColor * texture * blendFactor
	} blendMode;

	// the source texture is blended with this color [rgba] according to blendMode and blendFactor:
	glm::vec4 blendColor {1.f, 0.f, 0.f, 1.f};

	// blendFactor between blendColor and texture data : 0.0 means 100% blendColor, 1.0 means 100% texture data
	float blendFactor = 1.f;

	// if this is set to true, blendFactor is multiplied by source texture's alpha value at each pixel
	bool multiplyByTextureAlpha = true;
};

struct PictureVertex;

/* Renders pictures (gl textures) in 2D viewport space */
class PictureDraw {
public:
	static PictureDraw* get();
	virtual ~PictureDraw();

	// flush - all pending draw commands will be executed and all following commands
	// will produce graphics that will be layered on top of everything previous.
	// This is useful when interleaving draw calls with other 2D drawing code to achieve layering (such as with GLText)
	void flush();

	// draws a texture in viewport coordinates
	void draw(int texId, glm::vec2 pos, glm::vec2 size);

	// draws a texture with advanced blending
	void draw(int texId, glm::vec2 pos, glm::vec2 size, BlendOperation blendOp);

private:
	friend class RenderHelpers;
	static void init();
	static void unload();
	PictureDraw();

	struct RenderData;
	RenderData *pRenderData = nullptr;

	std::vector<PictureVertex> verts_;
	std::vector<int> texIds_;
	std::vector<BlendOperation> blendOps_;
};

#endif // BOGLFW_PICTURE_DRAW_H
