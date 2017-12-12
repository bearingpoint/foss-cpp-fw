#ifndef __glToolkit_h__
#define __glToolkit_h__

#define GLEW_NO_GLU
#include <GL/glew.h>

class GLFWwindow;

// initializes openGL an' all
bool gltInit(unsigned windowWidth=512, unsigned windowHeight=512, const char windowTitle[]="Untitled");

// begins a frame
void gltBegin();

// finishes a frame and displays the result
void gltEnd();

GLFWwindow* gltGetWindow();

#endif //__glToolkit_h__
