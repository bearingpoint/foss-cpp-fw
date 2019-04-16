#ifndef __RENDER_HELPERS_H__
#define __RENDER_HELPERS_H__

#include <string>

class Viewport;

/* This class manages initialization and destruction of the render helpers provided by the framework.
 * It also offers a convenience method to flush all helpers pending commands to the pipeline.
 * The helpers managed by this are:
 * * Shape2D
 * * Shape3D
 * * GLText
 * * MeshRenderer
 * * PictureDraw
 */
class RenderHelpers {
public:
	struct Config {
		std::string fontPath { "data/fonts/DejaVuSans.desc" }; // path to font *.desc file
	};

	static Config defaultConfig() { return Config{}; }

	// load and initialize render helpers (GLText, Shape2D, Shape3D etc)
	static void load(Config config = defaultConfig());
	// unload render helpers
	static void unload();

	// flushes all render helpers. This is automatically called by the Viewport after drawing everything, so
	// it's rarely required to call it directly.
	static void flushAll();

	// returns the viewport for which rendering is currently on-going, or nullptr if rendering is not in progress.
	static Viewport* getActiveViewport() { return pActiveViewport; }

private:
	friend class Viewport;
	static Viewport* pActiveViewport;
};

#endif // __RENDER_HELPERS_H__
