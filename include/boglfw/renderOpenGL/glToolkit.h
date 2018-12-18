#ifndef __glToolkit_h__
#define __glToolkit_h__

#define GLEW_NO_GLU
#include <GL/glew.h>
#include <glm/vec4.hpp>

class GLFWwindow;
class SDL_Window;

// initializes openGL an' all
bool gltInit(unsigned windowWidth=512, unsigned windowHeight=512, const char windowTitle[]="Untitled");

// begins a frame
void gltBegin(glm::vec4 clearColor = glm::vec4{0});

// finishes a frame and displays the result
void gltEnd();

GLFWwindow* gltGetWindow();

// initialize openGL on an SDL window
bool gltInitWithSDL(SDL_Window* window);

// checks if an OpenGL error has occured and prints it on stderr if so;
// returns true if error, false if no error
bool checkGLError();

#endif //__glToolkit_h__
