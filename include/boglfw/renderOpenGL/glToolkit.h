#ifndef __glToolkit_h__
#define __glToolkit_h__

#define GLEW_NO_GLU
#include <GL/glew.h>
#include <glm/vec4.hpp>

class GLFWwindow;
class SDL_Window;

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

// initializes openGL an' all
bool gltInit(unsigned windowWidth=512, unsigned windowHeight=512, const char windowTitle[]="Untitled");

// initializes openGL and create a supersampled framebuffer
bool gltInitSupersampled(unsigned windowWidth, unsigned windowHeight, SSDescriptor desc, const char windowTitle[]="Untitled");

// begins a frame
void gltBegin(glm::vec4 clearColor = glm::vec4{0});

// finishes a frame and displays the result
void gltEnd();

GLFWwindow* gltGetWindow();

// initialize openGL on an SDL window
bool gltInitWithSDL(SDL_Window* window);

// initialize openGL on an SDL window and create a supersampled framebuffer
bool gltInitWithSDLSupersampled(SDL_Window* window, SSDescriptor desc);

// returns true if SS is enabled and fills the provided buffer with data; returns false otherwise
bool getSuperSampleInfo(SSDescriptor& outDesc);

// checks if an OpenGL error has occured and prints it on stderr if so;
// returns true if error, false if no error
bool checkGLError(const char* operationName = nullptr);

// checks if an SDL error has occured and prints it on stderr if so;
// returns true if error, false if no error
bool checkSDLError(const char* operationName = nullptr);

#endif //__glToolkit_h__
