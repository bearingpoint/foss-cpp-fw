#ifndef __RENDER_CONTEXT_H__
#define __RENDER_CONTEXT_H__

class Viewport;

class RenderContext {
public:
	RenderContext(Viewport& viewport);

	Viewport const& viewport;

	// derive your own context from this and add here whatever other data you need

private:
	RenderContext(RenderContext const&) = delete; // prevent copying
};

#endif // __RENDER_CONTEXT_H__
