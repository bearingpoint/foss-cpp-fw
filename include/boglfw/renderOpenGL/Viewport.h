#pragma once

#include <boglfw/renderOpenGL/drawable.h>
#include <boglfw/utils/FlexibleCoordinate.h>

#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

#include <string>
#include <vector>

class Camera;

class Viewport : public FlexibleCoordinateContext {
public:
	Viewport(int x, int y, int w, int h);
	virtual ~Viewport();

	glm::vec3 bkColor() const { return backgroundColor_; }
	void setBkColor(glm::vec3 const& c) { backgroundColor_ = glm::vec4(c, 1.f); }
	void setBkColor(glm::vec4 c) { backgroundColor_ = c; }
	Camera& camera() const { return *pCamera_; }
	int width() const { return viewportArea_.z; }
	int height() const { return viewportArea_.w; }
	float aspect() const { return (float)width() / height(); }
	glm::vec2 position() const { return glm::vec2(viewportArea_.x, viewportArea_.y); }
	bool isEnabled() const { return enabled_; }
	bool containsPoint(glm::vec2 const&p) const;
	/**
	 * returned vector: x-X, y-Y, z-Width, w-Height
	 */
	glm::vec4 screenRect() const {return viewportArea_; }
	glm::vec3 project(glm::vec3 point) const;
	glm::vec3 unproject(glm::vec3 point) const;
	// returns a transformation matrix from viewport space to device uniform space -> useful for drawing viewport-space stuff
	glm::mat4 viewport2Uniform() const { return mVieport2Uniform_; }

	void setEnabled(bool enabled) { enabled_ = enabled; }
	void setArea(int vpX, int vpY, int vpW, int vpH);

	long userData() { return userData_; }
	void setUserData(long data) { userData_ = data; }

	void clear();
	void render(drawable element, RenderContext const& ctx);
	void render(std::vector<drawable> const& list, RenderContext const& ctx);

	virtual glm::vec2 getFlexCoordContextSize() const override { return {width(), height()}; }

protected:
	friend class RenderContext;

	long userData_ = 0;
	glm::vec4 viewportArea_;
	Camera* pCamera_ = nullptr;
	bool enabled_ = true;
	glm::vec4 backgroundColor_ {0.f, 0.f, 0.f, 1.f};

	glm::mat4 mVieport2Uniform_;
	mutable glm::mat4 mPV_cache_ {1};
	mutable glm::mat4 mPV_inv_cache_ {1};

	void updateVP2UMat();
	void prepareRendering(RenderContext const& ctx);
	void resetRendering();
};
