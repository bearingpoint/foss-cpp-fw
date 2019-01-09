#include <boglfw/renderOpenGL/glToolkit.h>
#include <boglfw/renderOpenGL/shader.hpp>
#include <boglfw/utils/log.h>

#ifdef WITH_GLFW
#include <GLFW/glfw3.h>
#endif

#ifdef WITH_SDL
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_opengl.h>
#endif

#include <iostream>
using namespace std;

#include <math.h>
#include <cassert>

#ifdef WITH_GLFW
static GLFWwindow* window = NULL;
#endif
static bool boundToSDL = false;
#ifdef WITH_SDL
static SDL_Window* sdl_window = nullptr;
#endif
static unsigned windowW = 0;
static unsigned windowH = 0;

// variables for super-sampling:
static bool ss_enabled = false;
static unsigned ss_bufferW = 0;		// the full width of the SS frame buffer
static unsigned ss_bufferH = 0;		// the full height of the SS frame buffer
static unsigned ss_bufferVPW = 0;	// the "viewport" (usable) width of the SS frame buffer
static unsigned ss_bufferVPH = 0;	// the "viewport" (usable) height of the SS frame buffer
									// if forcePowerOfTwoFramebuffer is set, the "viewport" w/h may be different
									// from the full w/h when the latter are increased to the nearest power of two
static unsigned ss_framebuffer = 0;
static unsigned ss_texture = 0;
static unsigned ss_renderbuffer = 0;
static unsigned ss_shaderProgram = 0;
static unsigned ss_shaderUSampOffs = 0;	// sample offsets uniform location
static unsigned ss_shaderUTexture = 0;	// texture sampler uniform location
static float ss_sampleOffsets[8];
static unsigned ss_quadVAO = 0;
static SSDescriptor ss_descriptor;

#ifdef WITH_GLFW
GLFWwindow* gltGetWindow() {
	return window;
}
#endif

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

static void setupSSFramebuffer(SSDescriptor descriptor) {
	ss_descriptor = descriptor;

	ss_bufferVPW = windowW * descriptor.getLinearSampleFactor();
	ss_bufferVPH = windowH * descriptor.getLinearSampleFactor();

	ss_bufferW = ss_bufferVPW;
	ss_bufferH = ss_bufferVPH;

	if (descriptor.forcePowerOfTwoFramebuffer) {
		// TODO - implement power of two framebuffer
		//... create the smallest pow-of-two framebuffer that is at least the required size
		//... then set the viewport only on the used region
		ERROR("Power-of-two framebuffer size not implemented. IGNORING SSDescriptor.forcePowerOfTwoFramebuffer field.");
	}

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

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	if (checkGLError("supersample framebuffer setup")
		|| glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		ERROR("Unable to setup a supersampled framebuffer, falling back to the default one.");
		return; // continue without supersampling
	}

	// load shader for post-render blit
	switch(ss_descriptor.mode) {
		case SSDescriptor::SS_4X:
			ss_shaderProgram = Shaders::createProgram("data/shaders/ssaa.vert", "data/shaders/ssaa.frag");
			break;
		case SSDescriptor::SS_9X:
		case SSDescriptor::SS_16X:
			ss_shaderProgram = Shaders::createProgram("data/shaders/ssaa.vert", "data/shaders/ssaa4s.frag");
			break;
		default:
			assert("Invalid super sampling mode!");
			return;
	}
	if (!ss_shaderProgram) {
		ERROR("Could not load super sampling shaders!");
		return;
	}
	unsigned posAttrIndex = glGetAttribLocation(ss_shaderProgram, "pos");
	unsigned uvAttrIndex = glGetAttribLocation(ss_shaderProgram, "uv");
	ss_shaderUSampOffs = glGetUniformLocation(ss_shaderProgram, "sampleOffsets");
	ss_shaderUTexture = glGetUniformLocation(ss_shaderProgram, "frameBufferTexture");

	// create screen quad:
	float screenQuadUV[] {
		0.f,
		0.f,
		ss_bufferVPW / (float)ss_bufferW,
		ss_bufferVPH / (float)ss_bufferH,
	};
	float screenQuadPosUV[] {
		-1.f, -1.f, screenQuadUV[0], screenQuadUV[1], 	// bottom-left
		-1.f, +1.f, screenQuadUV[0], screenQuadUV[3], 	// top-left
		+1.f, +1.f, screenQuadUV[2], screenQuadUV[3], 	// top-right
		+1.f, -1.f, screenQuadUV[2], screenQuadUV[1], 	// bottom-right
	};
	uint16_t screenQuadIdx[] {
		0, 1, 2, 0, 2, 3
	};
	unsigned quadVBO = 0;
	unsigned quadIBO = 0;
	glGenVertexArrays(1, &ss_quadVAO);
	glBindVertexArray(ss_quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(screenQuadPosUV), screenQuadPosUV, GL_STATIC_DRAW);
	glEnableVertexAttribArray(posAttrIndex);
	glVertexAttribPointer(posAttrIndex, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, 0);
	glEnableVertexAttribArray(uvAttrIndex);
	glVertexAttribPointer(uvAttrIndex, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, (void*)(sizeof(float)*2));
	glGenBuffers(1, &quadIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(screenQuadIdx), screenQuadIdx, GL_STATIC_DRAW);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(posAttrIndex);
	glDisableVertexAttribArray(uvAttrIndex);

	// compute sample offsets - offsets are considered from the pixel's default UV coordinates:
	// (we use built-in linear interpolation to reduce the number of sample points needed)
	float htU = 0.5f / ss_bufferW;	// half-texel U
	float htV = 0.5f / ss_bufferH;	// half-texel V
	if (descriptor.mode == SSDescriptor::SS_4X) {
		/* nothing to do here, the default sample point is already correctly positioned at the intersection of the 4 texels
		 * +---+---+--
		 * | 0 | 0 |
		 * +---S---+--
		 * | 0 | 0 |
		 * +---+---+--
		 * |   |   |
		 */
	} else if (descriptor.mode == SSDescriptor::SS_9X) {
		/* 4-sample pattern for 3x3 super samples:
		 * +---+---+---+--
		 * | 0 | 0 | 1 |
		 * +---S---+-S-+--
		 * | 0 | 0 | 1 |
		 * +---+---+---+--
		 * | S3| 2 S 2 |
		 * +---+---+---+--
		 * |   |   |   |
		 */
		ss_sampleOffsets[0] = -htU;		// #0
		ss_sampleOffsets[1] = -htV;
		ss_sampleOffsets[2] = +htU * 2;	// #1
		ss_sampleOffsets[3] = -htV;
		ss_sampleOffsets[4] = +htU;		// #2
		ss_sampleOffsets[5] = +htV * 2;
		ss_sampleOffsets[6] = -htU * 2;	// #3
		ss_sampleOffsets[7] = +htV * 2;
	} else if (descriptor.mode == SSDescriptor::SS_16X) {
		// 4 sample points, each in the middle of the 4x4 quarters
		ss_sampleOffsets[0] = -htU * 2;	// #0
		ss_sampleOffsets[1] = -htV * 2;
		ss_sampleOffsets[2] = +htU * 2;	// #1
		ss_sampleOffsets[3] = -htV * 2;
		ss_sampleOffsets[4] = +htU * 2;	// #2
		ss_sampleOffsets[5] = +htV * 2;
		ss_sampleOffsets[6] = -htU * 2;	// #3
		ss_sampleOffsets[7] = +htV * 2;
	}

	ss_enabled = true;
}

#ifdef WITH_GLFW
// initializes GLFW, openGL an' all
bool gltInitGLFW(unsigned windowWidth, unsigned windowHeight, const char windowTitle[]) {
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
		
	// disable vsync, use 1 to enable it
	glfwSwapInterval(0);

	windowW = windowWidth;
	windowH = windowHeight;

	return initGLEW();
}

bool gltInitGLFWSupersampled(unsigned windowWidth, unsigned windowHeight, SSDescriptor descriptor, const char windowTitle[]) {
	if (!gltInitGLFW(windowWidth, windowHeight, windowTitle))
		return false;
	setupSSFramebuffer(descriptor);
	return true;
}
#endif // WITH_GLFW

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
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
//	glBindFramebuffer(GL_READ_FRAMEBUFFER, ss_framebuffer);
//	glBlitFramebuffer(0, 0, ss_bufferW, ss_bufferH, 0, 0, windowW, windowH, GL_COLOR_BUFFER_BIT, GL_LINEAR);
//	return;
	glViewport(0, 0, windowW, windowH);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ss_texture);
	glUseProgram(ss_shaderProgram);
	glBindVertexArray(ss_quadVAO);
	glUniform2fv(ss_shaderUSampOffs, 4, ss_sampleOffsets);
	glUniform1i(ss_shaderUTexture, 0);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
	glBindVertexArray(0);
}

// finishes a frame and displays the result
void gltEnd() {
	LOGPREFIX("gltEnd");
	if (ss_enabled) {
		ssFBToScreen(); // render the off-screen framebuffer to the display backbuffer
	}
#ifdef WITH_SDL
	if (boundToSDL)
		SDL_GL_SwapWindow(sdl_window);
#endif
#ifdef WITH_GLFW
	if (!boundToSDL)
		glfwSwapBuffers(window);
#endif
	checkGLError("swap buffers");
}

#ifdef WITH_SDL
bool gltInitSDL(SDL_Window* window) {
	assert(window);
	sdl_window = window;
	SDL_GetWindowSize(window, (int*)&windowW, (int*)&windowH);
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

bool gltInitSDLSupersampled(SDL_Window* window, SSDescriptor descriptor) {
	if (!gltInitSDL(window))
		return false;
	setupSSFramebuffer(descriptor);
	return true;
}
#endif

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

#ifdef WITH_SDL
bool checkSDLError(const char* operationName) {
	auto err = SDL_GetError();
	if (err[0]) {
		ERROR("SDL error in [" << (operationName ? operationName : "") << "]:");
		ERROR(err);
		return true;
	} else
		return false;
}
#endif
