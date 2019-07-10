#ifndef __glToolkit_h__
#define __glToolkit_h__

#include <boglfw/utils/assert.h>

#define GLEW_NO_GLU
#include <GL/glew.h>
#include <glm/vec4.hpp>

#include <functional>

#if defined(WITH_SDL)
#elif defined(WITH_GLFW)
#else
#error "Neither WITH_GLFW nor WITH_SDL specified - no available windowing support!"
#endif

#ifdef WITH_GLFW
class GLFWwindow;
#endif

#ifdef WITH_SDL
class SDL_Window;
#endif

// describes a super-sampled framebuffer
struct SSDescriptor {
	enum {
		SS_4X,	// 2x2
		SS_9X,	// 3x3
		SS_16X	// 4x4
	} mode;		// select a supersampling mode
	bool forcePowerOfTwoFramebuffer = false;	// true to force create a framebuffer that has power-of-two width and height;
												// use this if the runtime system doesn't support non-power-of-two render targets
	unsigned framebufferFormat = GL_RGB8;		// the internal pixel format to use for the render target

	// returns the linear super sampling factor (how many samples per pixel in X or Y direction)
	unsigned getLinearSampleFactor() const {
		if (mode == SS_4X)
			return 2;
		else if (mode == SS_9X)
			return 3;
		else if (mode == SS_16X)
			return 4;
		else {
			assertDbg("invalid super sampling mode!");
			return 1;
		}
	}
};

struct GLFW_Init_Config {
	// the properties passed to the constructor are mandatory, while
	// all others are optional and have default values that fit general purpose use
	GLFW_Init_Config(unsigned winW, unsigned winH, const char* winTitle)
		: windowWidth(winW), windowHeight(winH), windowTitle(winTitle) {
	}
	
	unsigned windowWidth;
	unsigned windowHeight;
	const char* windowTitle;
	
	unsigned GL_Context_Major = 3;
	unsigned GL_Context_Minor = 0;
	bool GL_Context_Core_Profile = false;
	
	unsigned multiSampleCount = 0;
	bool createDepthStencilBuffer = false;
	unsigned depthBufferBits = 24;
	unsigned stencilBufferBits = 8;
	bool enableVSync = false;
	bool enableSuperSampling = false;
	SSDescriptor superSamplingConfig;
};

enum class PostProcessStep {
	PRE_DOWNSAMPLING,
	POST_DOWNSAMPLING
};

#ifdef WITH_GLFW
// initializes GLFW, openGL an' all
bool gltInitGLFW(GLFW_Init_Config cfg);

GLFWwindow* gltGetWindow();
#endif

#ifdef WITH_SDL
// initialize openGL on an SDL window
bool gltInitSDL(SDL_Window* window);

// initialize openGL on an SDL window and create a supersampled framebuffer (SSAA)
bool gltInitSDLSupersampled(SDL_Window* window, SSDescriptor desc);
#endif

void gltShutDown();

// begins a frame
void gltBegin(glm::vec4 clearColor = glm::vec4{0});

// finishes a frame and displays the result
void gltEnd();


// returns true if SS is enabled and fills the provided buffer with data; returns false otherwise
bool gltGetSuperSampleInfo(SSDescriptor& outDesc);

// Allows the caller to set up to two post-processing hooks, one that is executed before the downsampling step
// (on the raw super-sampled framebuffer - if supersampling is enabled),
// and the second after downsampling, on the screen-sized framebuffer.
// If supersampling is turned off, the pre-downsample hook will have no effect.
// In the pre-downsampling step, the draw framebuffer will have the same size as the supersampled framebuffer.
// In the post-downsampling step, the draw framebuffer has the same size as the screen framebuffer.
// Usually it's preferable to do postprocessing only after downsampling since it will be faster (fewer pixels), and
// it consumes less memory (no additional full-supersampled framebuffer needs to be created).
//
// [multisamples] specifies the number of multisamples (or zero to disable) to use for the off-screen framebuffer that
// the scene will be rendered onto. This is only used for post-downsampling step when supersampling is disabled, otherwise
// it is ignored. The texture that will be fed into the postprocessing callback is not multisampled, the multisamples are
// resolved prior to the call.
//
// The framebuffer texture to be used as input is bound to GL_TEXTURE0 GL_TEXTURE_2D target before the callback is invoked.
// The viewport is also correctly set to cover the entire target framebuffer prior to calling the callback.
// The user is responsible for all other aspects of the post-process rendering - screen quad, shader etc.
void gltSetPostProcessHook(PostProcessStep step, std::function<void()> hook, unsigned multisamples);

// checks if an OpenGL error has occured and prints it on stderr if so;
// returns true if error, false if no error
bool checkGLError(const char* operationName = nullptr);

#ifdef WITH_SDL
// checks if an SDL error has occured and prints it on stderr if so;
// returns true if error, false if no error
bool checkSDLError(const char* operationName = nullptr);
#endif

#endif //__glToolkit_h__
