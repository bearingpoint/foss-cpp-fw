#ifndef __RENDER_CONTEXT_H__
#define __RENDER_CONTEXT_H__

#include <boglfw/utils/assert.h>

class Viewport;

class RenderContext {
public:
	RenderContext() {}
	virtual ~RenderContext() {}

	Viewport const& viewport() const { assertDbg(pViewport != nullptr); return *pViewport; };

	mutable Viewport* pViewport = nullptr;

	// derive your own context from this and add here whatever other data you need

private:
	RenderContext(RenderContext const&) = delete; // prevent copying
};

#endif // __RENDER_CONTEXT_H__
