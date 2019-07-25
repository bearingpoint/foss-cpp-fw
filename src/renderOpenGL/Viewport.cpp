#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/Camera.h>
#include <boglfw/renderOpenGL/glToolkit.h>
#include <boglfw/renderOpenGL/RenderHelpers.h>
#include <boglfw/renderOpenGL/RenderContext.h>
#include <boglfw/utils/log.h>

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

Viewport::Viewport(int x, int y, int w, int h)
	: userData_(0)
	, viewportArea_(x, y, w, h)
	, pCamera_(new Camera(this))
	, enabled_(true)
{
	updateVP2UMat();
	pCamera_->moveTo({0, 0, -1});
	pCamera_->updateProj();
}

Viewport::~Viewport()
{
	delete pCamera_;
}

void Viewport::setArea(int vpX, int vpY, int vpW, int vpH)
{
	viewportArea_ = vec4(vpX, vpY, vpW, vpH);

	updateVP2UMat();
	pCamera_->updateProj();
}

void Viewport::updateVP2UMat() {
	int vpw = width(), vph = height();
	float sx = 2.f / (vpw-1);
	float sy = -2.f / (vph-1);
	float sz = -1.e-2f;
	glm::mat4x4 matVP_to_UniformScale(glm::scale(glm::mat4(1), glm::vec3(sx, sy, sz)));
	mVieport2Uniform_ = glm::translate(matVP_to_UniformScale,
			glm::vec3(-vpw/2, -vph/2, 0));
}

vec3 Viewport::unproject(vec3 point) const
{
	vec4 unif {point, 1};
	unif.x = unif.x / viewportArea_.z * 2 - 1;
	unif.y = 1 - unif.y / viewportArea_.w * 2;

	auto camPV = camera().matProjView();
	if (mPV_cache_ != camPV) {
		mPV_cache_ = camPV;
		mPV_inv_cache_ = glm::inverse(camPV);
	}

	auto ret = mPV_inv_cache_ * unif;
	return {ret.x, ret.y, ret.z};
}

vec3 Viewport::project(vec3 point) const
{
	auto matPV = camera().matProjView();
	auto unif = matPV * vec4{point, 1};
	vec3 ret { unif.x, unif.y, unif.z };
	ret *= 1.f / unif.w;
	ret.x *= viewportArea_.z * 0.5f;
	ret.y *= -viewportArea_.w * 0.5f;
	return ret + glm::vec3{viewportArea_.z * 0.5f, viewportArea_.w * 0.5f, 0};
}

bool Viewport::containsPoint(glm::vec2 const&p) const {
	return p.x >= viewportArea_.x && p.y >= viewportArea_.y &&
			p.x <= viewportArea_.x + viewportArea_.z &&
			p.y <= viewportArea_.y + viewportArea_.w;
}

void Viewport::clear() {
	SSDescriptor ssDesc;
	bool ssEnabled = gltGetSuperSampleInfo(ssDesc);
	// when super sample is enabled we must adjust the viewport accordingly
	unsigned vpfx = ssEnabled ? ssDesc.getLinearSampleFactor() : 1;
	unsigned vpfy = ssEnabled ? ssDesc.getLinearSampleFactor() : 1;
	auto vpp = position();
	glScissor(vpp.x * vpfx, vpp.y * vpfy, width() * vpfx, height() * vpfy);
	glEnable(GL_SCISSOR_TEST);
	glClearColor(backgroundColor_.r, backgroundColor_.g, backgroundColor_.b, backgroundColor_.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);
	checkGLError("viewport clear");
}

void Viewport::prepareRendering(RenderContext const& ctx) {
	// set up viewport:
	assertDbg(RenderHelpers::pActiveViewport == nullptr && "Another viewport is already rendering");
	RenderHelpers::pActiveViewport = this;

	SSDescriptor ssDesc;
	bool ssEnabled = gltGetSuperSampleInfo(ssDesc);
	// when super sample is enabled we must adjust the viewport accordingly
	unsigned vpfx = ssEnabled ? ssDesc.getLinearSampleFactor() : 1;
	unsigned vpfy = ssEnabled ? ssDesc.getLinearSampleFactor() : 1;
	auto vpp = position();
	glViewport(vpp.x * vpfx, vpp.y * vpfy, width() * vpfx, height() * vpfy);

	// make sure the context is refering to this viewport:
	ctx.pViewport = this;
}

void Viewport::resetRendering() {
	RenderHelpers::pActiveViewport = nullptr;
}

void Viewport::render(drawable element, RenderContext const& ctx) {
	if (!isEnabled())
		return;

	checkGLError("Viewport::render before prepare");
	prepareRendering(ctx);
	checkGLError("Viewport::render after prepare, before element.draw()");

	element.draw(ctx);
	checkGLError("Viewport::render after element.draw()");
	// flush all render helpers' pending commands
	RenderHelpers::flushAll();
	checkGLError("Viewport::render after flushAll()");

	resetRendering();
	checkGLError("Viewport::render end.");
}

void Viewport::render(std::vector<drawable> const& list, RenderContext const& ctx) {
	if (!isEnabled())
		return;

	checkGLError("Viewport::render before prepare");
	prepareRendering(ctx);
	checkGLError("Viewport::render after prepare, before element.draw()");

	// render objects from list:
	for (auto &x : list) {
		x.draw(ctx);
		checkGLError("Viewport::render after element.draw()");
	}
	// flush all render helpers' pending commands
	RenderHelpers::flushAll();
	checkGLError("Viewport::render after flushAll()");

	resetRendering();
	checkGLError("Viewport::render end.");
}
