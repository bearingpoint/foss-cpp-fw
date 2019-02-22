#ifndef __RENDER_HELPERS_H__
#define __RENDER_HELPERS_H__

#include <string>

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
};

#endif // __RENDER_HELPERS_H__
