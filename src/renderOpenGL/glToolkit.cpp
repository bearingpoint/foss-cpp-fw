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
SDL_Window* sdl_window = nullptr;

GLFWwindow* gltGetWindow() {
	return window;
}

bool initGLEW() {
	glewExperimental = GL_TRUE;
	if (int glewError = glewInit() != GLEW_OK) {
		cerr << "FAILED glewInit" << endl;
		cerr << "\"" << glewGetErrorString(glewError) << "\"" << endl;
		return false;
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	checkGLError("init GLEW");
	return true;
}

// initializes openGL an' all
bool gltInit(unsigned windowWidth, unsigned windowHeight, const char windowTitle[])
{
	// initialize GLFW and set-up window an' all:
	if (!glfwInit()) {
		cerr << "FAILED glfwInit" << endl;
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	//glfwWindowHint(GLFW_DEPTH_BITS, 24);
	//glfwWindowHint(GLFW_STENCIL_BITS, 8);

	window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, NULL, NULL);
	if (!checkGLError("glfwCreateWindow"))
		return false;
	if (!window) {
		cerr << "FAILED creating window" << endl;
		return false;
	}
	glfwMakeContextCurrent(window);
	if (!checkGLError("glfwMakeContextCurrent"))
		return false;

	return initGLEW();
}

// begins a frame
void gltBegin(glm::vec4 clearColor)
{
	LOGPREFIX("gltBegin");
	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	checkGLError("clear");
}

// finishes a frame and displays the result
void gltEnd()
{
	LOGPREFIX("gltEnd");
	if (boundToSDL)
		SDL_GL_SwapWindow(sdl_window);
	else
		glfwSwapBuffers(window);
	checkGLError("swap buffers");
}

bool gltInitWithSDL(SDL_Window* window) {
	assert(window);
	sdl_window = window;
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	// SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE); // only set this for >=3.2 profiles using VAOs, not user memory
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
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
