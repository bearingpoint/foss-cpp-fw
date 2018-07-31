#include <boglfw/renderOpenGL/glToolkit.h>
#include <boglfw/renderOpenGL/shader.hpp>

#define GLFW_DLL
#include <GLFW/glfw3.h>


#include <iostream>
using namespace std;

#include <math.h>

static GLFWwindow* window = NULL;

GLFWwindow* gltGetWindow() {
	return window;
}

// initializes openGL an' all
bool gltInit(unsigned windowWidth, unsigned windowHeight, const char windowTitle[])
{
	// initialize GLFW and set-up window an' all:
	if (!glfwInit()) {
		cout << "FAILED glfwInit" << endl;
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	//glfwWindowHint(GLFW_DEPTH_BITS, 24);
	//glfwWindowHint(GLFW_STENCIL_BITS, 8);

	window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, NULL, NULL);
	if (!window) {
		cout << "FAILED creating window" << endl;
		return false;
	}
	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	if (int glewError = glewInit() != GLEW_OK) {
		cout << "FAILED glewInit" << endl;
		cout << "\"" << glewGetErrorString(glewError) << "\"" << endl;
		return false;
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	return true;
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

