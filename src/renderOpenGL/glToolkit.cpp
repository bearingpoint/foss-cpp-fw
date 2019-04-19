#include <boglfw/renderOpenGL/glToolkit.h>
#include <boglfw/renderOpenGL/shader.h>
#include <boglfw/renderOpenGL/Framebuffer.h>
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
static unsigned defaultMultisamples = 0;

// variables for super-sampling:
static bool ss_enabled = false;
static unsigned ss_bufferVPW = 0;	// the "viewport" (usable) width of the SS frame buffer
static unsigned ss_bufferVPH = 0;	// the "viewport" (usable) height of the SS frame buffer
									// if forcePowerOfTwoFramebuffer is set, the "viewport" w/h may be different
									// from the full w/h when the latter are increased to the nearest power of two
static unsigned ss_shaderProgram = 0;
static unsigned ss_shaderUSampOffs = 0;	// sample offsets uniform location
static unsigned ss_shaderUTexture = 0;	// texture sampler uniform location
static float ss_sampleOffsets[8];
static unsigned ss_quadVAO = 0;
static SSDescriptor ss_descriptor;
static FrameBufferDescriptor ss_framebufferDesc;
static FrameBuffer ss_framebuffer;

static std::function<void()> postProcessHooks[2] { nullptr, nullptr };
static FrameBuffer pp_framebuffers[3]; // if using only post-downsampling pp and no supersampling, #1 needs a depth buffer

#ifdef WITH_GLFW
GLFWwindow* gltGetWindow() {
	return window;
}
#endif

bool gltGetSuperSampleInfo(SSDescriptor& outDesc) {
	if (!ss_enabled)
		return false;
	int crtFramebuffer;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &crtFramebuffer);
	if (crtFramebuffer != ss_framebuffer.framebufferId())
		return false;	// a framebuffer other than the supersampled framebuffer is currently bound so we don't interfere
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
	glDepthFunc(GL_LEQUAL);
	checkGLError("init GLEW");
	return true;
}

static void setupSSFramebuffer(SSDescriptor descriptor) {
	ss_descriptor = descriptor;

	ss_bufferVPW = windowW * descriptor.getLinearSampleFactor();
	ss_bufferVPH = windowH * descriptor.getLinearSampleFactor();

	ss_framebufferDesc.width = ss_bufferVPW;
	ss_framebufferDesc.height = ss_bufferVPH;
	ss_framebufferDesc.format = ss_descriptor.framebufferFormat;
	ss_framebufferDesc.requireDepthBuffer = true;

	if (descriptor.forcePowerOfTwoFramebuffer) {
		// TODO - implement power of two framebuffer
		//... create the smallest pow-of-two framebuffer that is at least the required size
		//... then set the viewport only on the used region
		ERROR("Power-of-two framebuffer size not implemented. IGNORING SSDescriptor.forcePowerOfTwoFramebuffer field.");
	}

	// set up the super sampled framebuffer and attachments:
	if (!ss_framebuffer.create(ss_framebufferDesc)) {
		ERROR("Unable to create a supersampled framebuffer, falling back to the default one.");
		return; // continue without supersampling
	}

	// load shader for post-render blit
	switch(ss_descriptor.mode) {
		case SSDescriptor::SS_4X:
			Shaders::createProgram("data/shaders/ssaa.vert", "data/shaders/ssaa.frag", [&](unsigned id) {
				ss_shaderProgram = id;
			});
			break;
		case SSDescriptor::SS_9X:
		case SSDescriptor::SS_16X:
			Shaders::createProgram("data/shaders/ssaa.vert", "data/shaders/ssaa4s.frag", [&](unsigned id) {
				ss_shaderProgram = id;
			});
			break;
		default:
			assertDbg("Invalid super sampling mode!");
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
		ss_bufferVPW / (float)ss_framebufferDesc.width,
		ss_bufferVPH / (float)ss_framebufferDesc.height,
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

	// compute sample offsets - offsets are considered from the pixel's default UV coordinates:
	// (we use built-in linear interpolation to reduce the number of sample points needed)
	float htU = 0.5f / ss_framebufferDesc.width;	// half-texel U
	float htV = 0.5f / ss_framebufferDesc.height;	// half-texel V
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
bool gltInitGLFW(unsigned windowWidth, unsigned windowHeight, const char windowTitle[],
					unsigned multiSampleCount, bool createDepthStencilBuffer) {
	// initialize GLFW and set-up window an' all:
	if (!glfwInit()) {
		cerr << "FAILED glfwInit" << endl;
		return false;
	}
	defaultMultisamples = multiSampleCount;

	glfwWindowHint(GLFW_SAMPLES, defaultMultisamples);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	if (createDepthStencilBuffer) {
		glfwWindowHint(GLFW_DEPTH_BITS, 24);
		glfwWindowHint(GLFW_STENCIL_BITS, 8);
	}

	window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, NULL, NULL);
	if (!window) {
		cerr << "FAILED creating window" << endl;
		return false;
	}
	glfwMakeContextCurrent(window);
	if (checkGLError("glfwMakeContextCurrent"))
		return false;

	// disable vsync, use 1 to enable it
	glfwSwapInterval(1);

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
	checkGLError("before gltBegin()");
	if (ss_enabled)
		ss_framebuffer.bind();
	else if (postProcessHooks[1])
		pp_framebuffers[1].bind();
	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	checkGLError("clear");
}

static void ssFBToScreen() {
	if (postProcessHooks[1])
		pp_framebuffers[1].bind();
	else
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glDisable(GL_DEPTH_TEST);

	glViewport(0, 0, windowW, windowH);
	glActiveTexture(GL_TEXTURE0);
	if (postProcessHooks[0])
		glBindTexture(GL_TEXTURE_2D, pp_framebuffers[0].fbTextureId());	// read from the post-process target
	else
		glBindTexture(GL_TEXTURE_2D, ss_framebuffer.fbTextureId());		// read from the supersampled framebuffer directly
	glUseProgram(ss_shaderProgram);
	glBindVertexArray(ss_quadVAO);
	glUniform2fv(ss_shaderUSampOffs, 4, ss_sampleOffsets);
	glUniform1i(ss_shaderUTexture, 0);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
	glBindVertexArray(0);

	glEnable(GL_DEPTH_TEST);

	if (postProcessHooks[1])
		pp_framebuffers[1].unbind();
}

// finishes a frame and displays the result
void gltEnd() {
	LOGPREFIX("gltEnd");
	checkGLError("before gltEnd()");

	if (ss_enabled) {
		ss_framebuffer.unbind();
		if (postProcessHooks[0]) {
			// pre-downsampling post processing step
			glDisable(GL_DEPTH_TEST);
			pp_framebuffers[0].bind();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, ss_framebuffer.fbTextureId());
			glViewport(0, 0, ss_framebufferDesc.width, ss_framebufferDesc.height);
			postProcessHooks[0]();
			glEnable(GL_DEPTH_TEST);
			pp_framebuffers[0].unbind();
		}
		ssFBToScreen(); // render the off-screen framebuffer to the display/post-process backbuffer
		checkGLError("gltEnd() -> ssFBToScreen()");
	}
	if (postProcessHooks[1]) {
		pp_framebuffers[1].unbind();
		glDisable(GL_DEPTH_TEST);
		glActiveTexture(GL_TEXTURE0);
		if (pp_framebuffers[2].valid()) {
			// we used a multisampled framebuffer to render the scene, must resolve it before post-processing
			pp_framebuffers[2].bind(); //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, pp_framebuffers[2].frameBufferId);
			pp_framebuffers[1].bindRead(); //glBindFramebuffer(GL_READ_FRAMEBUFFER, pp_framebuffers[1].frameBufferId);
			glBlitFramebuffer(0, 0, windowW, windowH, 0, 0, windowW, windowH, GL_COLOR_BUFFER_BIT, GL_NEAREST);
			pp_framebuffers[1].unbindRead();
			glBindTexture(GL_TEXTURE_2D, pp_framebuffers[2].fbTextureId());
			pp_framebuffers[2].unbind();
		} else {
			glBindTexture(GL_TEXTURE_2D, pp_framebuffers[1].fbTextureId());
		}
		checkGLError("gltEnd() PostProcess #1");
		// post-downsampling post-processing step into the default screen framebuffer
		glViewport(0, 0, windowW, windowH);
		postProcessHooks[1]();
		glEnable(GL_DEPTH_TEST);
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
	assertDbg(window);
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

void gltSetPostProcessHook(PostProcessStep step, std::function<void()> hook, unsigned multisampleFb) {
	if (step == PostProcessStep::PRE_DOWNSAMPLING && !ss_enabled) {
		ERROR("Pre-downsampling postprocessing hook requested, but SSAA is not active.");
		return;	// can't do pre-downsampling post-processing if supersampling is disabled
	}
	postProcessHooks[step == PostProcessStep::PRE_DOWNSAMPLING ? 0 : 1] = hook;

	// now check if we need to create or destroy additional framebuffers:
	if (postProcessHooks[0]) {
		if (!pp_framebuffers[0].valid()) {
			// post-processing enabled, need to create additional framebuffer
			FrameBufferDescriptor desc = ss_framebufferDesc;
			desc.format = GL_RGB16;
			desc.requireDepthBuffer = false;
			if (!pp_framebuffers[0].create(desc)) {
				ERROR("Failed to create additional framebuffer for pre-downsampling post-processing; disabling post-processing at this step.");
				postProcessHooks[0] = nullptr;
			}
		}
	} else if (pp_framebuffers[0].valid()) {
		// post-processing disabled, destroy additional framebuffer
		pp_framebuffers[0].destroy();
	}
	if (postProcessHooks[1]) {
		if (!pp_framebuffers[1].valid()) {
			if (defaultMultisamples > 0) {
				ERROR("WARNING: Creating a post-processing hook while the Default Framebuffer is multisampled. "
						"Disable the default multisampling to stop wasting memory (it has no effect).");
			}
			if (multisampleFb > 0 && ss_enabled) {
				ERROR("WARNING: Specified multisamples for post-processing framebuffer while SSAA is active. Ignoring multisample param");
				multisampleFb = 0;
			}
			// post-processing enabled, need to create additional framebuffer(s)
			if (multisampleFb > 0) {
				// must create one multisampled framebuffer and one additional non-multisampled one to resolve it into
				FrameBufferDescriptor desc;
				desc.format = GL_RGB16;
				desc.height = windowH;
				desc.width = windowW;
				desc.multisamples = multisampleFb;
				desc.requireDepthBuffer = true;
				bool ok = pp_framebuffers[1].create(desc);
				desc.multisamples = 0;
				desc.requireDepthBuffer = false;
				ok &= pp_framebuffers[2].create(desc);
				if (!ok) {
					ERROR("Failed to create additional framebuffer for post-downsampling post-processing; disabling post-processing at this step.");
					postProcessHooks[1] = nullptr;
					if (pp_framebuffers[1].valid())
						pp_framebuffers[1].destroy();
					if (pp_framebuffers[2].valid())
						pp_framebuffers[2].destroy();
				}
			} else {
				FrameBufferDescriptor desc;
				desc.format = GL_RGB16;
				desc.height = windowH;
				desc.width = windowW;
				desc.multisamples = 0;
				desc.requireDepthBuffer = !ss_enabled;
				if (!pp_framebuffers[1].create(desc)) {
					ERROR("Failed to create additional framebuffer for post-downsampling post-processing; disabling post-processing at this step.");
					postProcessHooks[1] = nullptr;
				}
			}
		}
	} else if (pp_framebuffers[1].valid()) {
		if (pp_framebuffers[2].valid()) {
			// hook #1 used multisampled framebuffer
			pp_framebuffers[1].destroy();
			pp_framebuffers[2].destroy();
		} else {
			// post-processign disabled, destroy additional framebuffer
			pp_framebuffers[1].destroy();
		}
	}
}
