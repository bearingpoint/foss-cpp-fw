#include <boglfw/renderOpenGL/glToolkit.h>
#include <boglfw/renderOpenGL/shader.hpp>

#include <GLFW/glfw3.h>

#include <SDL2/SDL_video.h>
#include <SDL2/SDL_opengl.h>

#include <iostream>
using namespace std;

#include <math.h>
#include <cassert>

static GLFWwindow* window = NULL;

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

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	//glfwWindowHint(GLFW_DEPTH_BITS, 24);
	//glfwWindowHint(GLFW_STENCIL_BITS, 8);

	window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, NULL, NULL);
	if (!window) {
		cerr << "FAILED creating window" << endl;
		return false;
	}
	glfwMakeContextCurrent(window);

	return initGLEW();
}

// begins a frame
void gltBegin()
{
	glClearColor(0,0,0,0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);	// ================================
}

// finishes a frame and displays the result
void gltEnd()
{
	glfwSwapBuffers(window);
}

bool gltInitWithSDL(SDL_Window* window) {
	assert(window);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	auto context = SDL_GL_CreateContext(window);
	if (!context)
		return false;
	SDL_GL_MakeCurrent(window, context);
	return initGLEW();
}
