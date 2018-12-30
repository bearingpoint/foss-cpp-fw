#include <boglfw/renderOpenGL/glToolkit.h>
#include <boglfw/renderOpenGL/shader.hpp>
#include <boglfw/utils/log.h>

#include <GLFW/glfw3.h>

#include <SDL2/SDL_video.h>
#include <SDL2/SDL_opengl.h>

#include <iostream>
using namespace std;

#include <math.h>
#include <cassert>

static GLFWwindow* window = NULL;
static bool boundToSDL = false;
static SDL_Window* sdl_window = nullptr;
static unsigned windowW = 0;
static unsigned windowH = 0;

// variables for super-sampling:
static bool ss_enabled = false;
static unsigned ss_bufferW = 0;
static unsigned ss_bufferH = 0;
static unsigned ss_framebuffer = 0;
static unsigned ss_texture = 0;
static unsigned ss_renderbuffer = 0;
static SSDescriptor ss_descriptor;

GLFWwindow* gltGetWindow() {
	return window;
}

bool getSuperSampleInfo(SSDescriptor& outDesc) {
	if (!ss_enabled)
		return false;
	outDesc = ss_descriptor;
	return true;
}

static bool initGLEW() {
	glewExperimental = GL_TRUE;
	if (int glewError = glewInit() != GLEW_OK) {
		cerr << "FAILED glewInit" << endl;
		cerr << "\"" << glewGetErrorString(glewError) << "\"" << endl;
		return false;
	}

	glEnable(GL_DEPTH_TEST);
//	glEnable(GL_MULTISAMPLE);
	glDepthFunc(GL_LEQUAL);
	checkGLError("init GLEW");
	return true;
}

// initializes openGL an' all
bool gltInit(unsigned windowWidth, unsigned windowHeight, const char windowTitle[]) {
	// initialize GLFW and set-up window an' all:
	if (!glfwInit()) {
		cerr << "FAILED glfwInit" << endl;
		return false;
	}

//	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_DEPTH_BITS, 24);
	glfwWindowHint(GLFW_STENCIL_BITS, 8);

	window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, NULL, NULL);
	if (!window) {
		cerr << "FAILED creating window" << endl;
		return false;
	}
	glfwMakeContextCurrent(window);
	if (checkGLError("glfwMakeContextCurrent"))
		return false;

	windowW = windowWidth;
	windowH = windowHeight;

	return initGLEW();
}

bool gltInitSupersampled(unsigned windowWidth, unsigned windowHeight, SSDescriptor descriptor, const char windowTitle[]) {
	if (!gltInit(windowWidth, windowHeight, windowTitle))
		return false;

	ss_descriptor = descriptor;

	if (descriptor.forcePowerOfTwoFramebuffer) {
		// TODO - implement power of two framebuffer
		//... create the smallest pow-of-two framebuffer that is at least the required size
		//... then set the viewport only on the used region
		ERROR("Power-of-two framebuffer size not implemented. IGNORING SSDescriptor.forcePowerOfTwoFramebuffer field.");
	}

	ss_bufferW = windowWidth * descriptor.fragments_xr;
	ss_bufferH = windowHeight * descriptor.fragments_yr;

	// set up the super sampled framebuffer and attachments:
	glGenFramebuffers(1, &ss_framebuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ss_framebuffer);

	glGenTextures(1, &ss_texture);
	glBindTexture(GL_TEXTURE_2D, ss_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, descriptor.framebufferFormat, ss_bufferW, ss_bufferH, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, ss_texture, 0);

	glGenRenderbuffers(1, &ss_renderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, ss_renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, ss_bufferW, ss_bufferH);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, ss_renderbuffer);

//	auto dbufs[] = {GL_COLOR_ATTACHMENT0};
//	glDrawBuffers(1, dbufs);

	// load shader for post-render blit

	if (checkGLError("supersample framebuffer setup")
		|| glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		ERROR("Unable to setup a supersampled framebuffer, falling back to the default one.");
		return true; // continue without supersampling
	}

	// adjust line rendering
	glLineWidth(0.5f * (descriptor.fragments_xr + descriptor.fragments_yr));
	glEnable(GL_LINE_SMOOTH);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	ss_enabled = true;
	return true;
}

// begins a frame
void gltBegin(glm::vec4 clearColor) {
	LOGPREFIX("gltBegin");
	if (ss_enabled)
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ss_framebuffer);
	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	checkGLError("clear");
}

static void ssFBToScreen() {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, ss_framebuffer);
	glBlitFramebuffer(0, 0, ss_bufferW, ss_bufferH, 0, 0, windowW, windowH, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

// finishes a frame and displays the result
void gltEnd() {
	LOGPREFIX("gltEnd");
	if (ss_enabled) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		ssFBToScreen(); // render the off-screen framebuffer to the display backbuffer
	}
	if (boundToSDL)
		SDL_GL_SwapWindow(sdl_window);
	else
		glfwSwapBuffers(window);
	checkGLError("swap buffers");
}

bool gltInitWithSDL(SDL_Window* window) {
	assert(window);
	sdl_window = window;
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, SDL_TRUE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE); // only set this for >=3.2 profiles using VAOs, not user memory
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	auto context = SDL_GL_CreateContext(window);
	if (checkSDLError("SDL_GL_CreateContext"))
		return false;
	if (!context)
		return false;
	SDL_GL_MakeCurrent(window, context);
	if (checkSDLError("SDL_GL_MakeCurrent"))
		return false;
	boundToSDL = true;
	return initGLEW();
}

bool checkGLError(const char* operationName) {
	bool errorDetected = false;
	GLenum err;
	do {
		err = glGetError();
		if (err != GL_NO_ERROR) {
			static char buf[32];
			snprintf(buf, sizeof(buf), "%#6x", err);
			ERROR("GL error in [" << (operationName ? operationName : "") << "] code " << buf);
			errorDetected = true;
		}
	} while (err != GL_NO_ERROR);
	return errorDetected;
}

bool checkSDLError(const char* operationName) {
	auto err = SDL_GetError();
	if (err[0]) {
		ERROR("SDL error in [" << (operationName ? operationName : "") << "]:");
		ERROR(err);
		return true;
	} else
		return false;
}
