#ifndef __glToolkit_h__
#define __glToolkit_h__

#define GLEW_NO_GLU
#include <GL/glew.h>
#include <glm/vec4.hpp>

class GLFWwindow;
class SDL_Window;

// describes a super-sampled framebuffer
struct SSDescriptor {
	unsigned fragments_xr;	// horizontal fragment ratio (fragments per pixel)
	unsigned fragments_yr;	// vertical fragment ratio (fragments per pixel)
	bool forcePowerOfTwoFramebuffer = false;	// true to force create a framebuffer that has power-of-two width and height;
												// use this if the runtime system doesn't support non-power-of-two render targets
	unsigned framebufferFormat = GL_RGB8;		// the internal pixel format to use for the render target
};

// initializes openGL an' all
bool gltInit(unsigned windowWidth=512, unsigned windowHeight=512, const char windowTitle[]="Untitled");

// initializes openGL an' all
bool gltInitSupersampled(unsigned windowWidth, unsigned windowHeight, SSDescriptor samples, const char windowTitle[]="Untitled");

// begins a frame
void gltBegin(glm::vec4 clearColor = glm::vec4{0});

// finishes a frame and displays the result
void gltEnd();

GLFWwindow* gltGetWindow();

// initialize openGL on an SDL window
bool gltInitWithSDL(SDL_Window* window);

// returns true if SS is enabled and fills the provided buffer with data; returns false otherwise
bool getSuperSampleInfo(SSDescriptor& outDesc);

// checks if an OpenGL error has occured and prints it on stderr if so;
// returns true if error, false if no error
bool checkGLError(const char* operationName = nullptr);

// checks if an SDL error has occured and prints it on stderr if so;
// returns true if error, false if no error
bool checkSDLError(const char* operationName = nullptr);

#endif //__glToolkit_h__
