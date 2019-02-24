#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/Camera.h>
#include <boglfw/renderOpenGL/glToolkit.h>
#include <boglfw/renderOpenGL/RenderHelpers.h>
#include <boglfw/renderOpenGL/RenderContext.h>
#include <boglfw/utils/log.h>

using namespace glm;

Viewport::Viewport(int x, int y, int w, int h)
	: userData_(0)
	, viewportArea_(x, y, w, h)
	, pCamera_(new Camera(this))
	, enabled_(true)
{
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

	pCamera_->updateProj();
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
	glScissor(vpp.x * vpfx, vpp.y * vpfx, width() * vpfx, height() * vpfx);
	glEnable(GL_SCISSOR_TEST);
	glClearColor(backgroundColor_.r, backgroundColor_.g, backgroundColor_.b, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);
	checkGLError("viewport clear");
}

void Viewport::render(std::vector<drawable> const& list) {
	if (!isEnabled())
		return;
	if (!pContext_) {
		ERROR("No RenderContext created for the viewport!");
		return;
	}
	// set up viewport:
	SSDescriptor ssDesc;
	bool ssEnabled = gltGetSuperSampleInfo(ssDesc);
	// when super sample is enabled we must adjust the viewport accordingly
	unsigned vpfx = ssEnabled ? ssDesc.getLinearSampleFactor() : 1;
	unsigned vpfy = ssEnabled ? ssDesc.getLinearSampleFactor() : 1;
	auto vpp = position();
	glViewport(vpp.x * vpfx, vpp.y * vpfy, width() * vpfx, height() * vpfy);

	RenderHelpers::pActiveViewport = this;

	// render objects from list:
	for (auto &x : list) {
		x.draw(*pContext_);
	}

	// flush all render helpers' pending commands
	RenderHelpers::flushAll();

	RenderHelpers::pActiveViewport = nullptr;
}
