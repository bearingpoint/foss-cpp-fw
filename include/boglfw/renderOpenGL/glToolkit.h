#ifndef __glToolkit_h__
#define __glToolkit_h__

#define GLEW_NO_GLU
#include <GL/glew.h>
#include <glm/vec4.hpp>

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
			assert("invalid super sampling mode!");
			return 1;
		}
	}
};

#ifdef WITH_GLFW
// initializes GLFW, openGL an' all
// if multisampleCount is non-zero, multi-sampling antialiasing (MSAA) will be enabled
bool gltInitGLFW(unsigned windowWidth=512, unsigned windowHeight=512, const char windowTitle[]="Untitled", unsigned multiSampleCount=0);

// initializes openGL and create a supersampled framebuffer (SSAA)
bool gltInitGLFWSupersampled(unsigned windowWidth, unsigned windowHeight, SSDescriptor desc, const char windowTitle[]="Untitled");

GLFWwindow* gltGetWindow();
#endif

#ifdef WITH_SDL
// initialize openGL on an SDL window
bool gltInitSDL(SDL_Window* window);

// initialize openGL on an SDL window and create a supersampled framebuffer (SSAA)
bool gltInitSDLSupersampled(SDL_Window* window, SSDescriptor desc);
#endif

// begins a frame
void gltBegin(glm::vec4 clearColor = glm::vec4{0});

// finishes a frame and displays the result
void gltEnd();


// returns true if SS is enabled and fills the provided buffer with data; returns false otherwise
bool getSuperSampleInfo(SSDescriptor& outDesc);

// checks if an OpenGL error has occured and prints it on stderr if so;
// returns true if error, false if no error
bool checkGLError(const char* operationName = nullptr);

#ifdef WITH_SDL
// checks if an SDL error has occured and prints it on stderr if so;
// returns true if error, false if no error
bool checkSDLError(const char* operationName = nullptr);
#endif

#endif //__glToolkit_h__
