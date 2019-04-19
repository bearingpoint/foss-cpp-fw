#include <boglfw/renderOpenGL/Framebuffer.h>
#include <boglfw/renderOpenGL/glToolkit.h>
#include <boglfw/utils/log.h>

bool FrameBufferDescriptor::validate() const {
	int acceptedFormats[] {
		GL_RGB,
		GL_RGBA,
		GL_RGB8,
		GL_RGBA8,
		GL_RGB16,
		GL_RGBA16,
		GL_RGB16F,
		GL_RGBA16F,
		GL_RGB32F,
		GL_RGBA32F,
		GL_R8,
		GL_R16,
		GL_R16F,
		GL_R32F,
		GL_RG8,
		GL_RG16,
		GL_RG16F,
		GL_RG32F
	};
	bool formatCorrect = false;
	for (int n=sizeof(acceptedFormats) / sizeof(acceptedFormats[0]), i=0; i<n; i++)
		if (this->format == acceptedFormats[i]) {
			formatCorrect = true;
			break;
		}
	if (!formatCorrect)
		return false;
	if (this->height > 8192)
		return false;
	if (this->width > 8192)
		return false;
	if (this->multisamples > 16)
		return false;

	return true;
}

GLuint FrameBufferDescriptor::textureChannels() const {
	switch (format) {
		case GL_RGB:
		case GL_RGB8:
		case GL_RGB16:
		case GL_RGB16F:
		case GL_RGB32F:
			return GL_RGB;

		case GL_RGBA:
		case GL_RGBA8:
		case GL_RGBA16:
		case GL_RGBA16F:
		case GL_RGBA32F:
			return GL_RGBA;

		case GL_R8:
		case GL_R16:
		case GL_R16F:
		case GL_R32F:
			return GL_RED;

		case GL_RG8:
		case GL_RG16:
		case GL_RG16F:
		case GL_RG32F:
			return GL_RG;
	}
}

GLuint FrameBufferDescriptor::textureDataType() const {
	switch (format) {
		case GL_RGB:
		case GL_RGB8:
		case GL_RGBA:
		case GL_RGBA8:
		case GL_R8:
		case GL_RG8:
			return GL_UNSIGNED_BYTE;

		case GL_R16:
		case GL_RG16:
		case GL_RGB16:
		case GL_RGBA16:
			return GL_UNSIGNED_SHORT;

		case GL_RGB16F:
		case GL_RGBA16F:
		case GL_R16F:
		case GL_RG16F:
			return GL_HALF_FLOAT;

		case GL_RGB32F:
		case GL_RGBA32F:
		case GL_R32F:
		case GL_RG32F:
			return GL_FLOAT;
	}
}

FrameBuffer::~FrameBuffer() {
	if (created_)
		destroy();
}

bool FrameBuffer::create(FrameBufferDescriptor const& desc) {
	if (created_) {
		assertDbg(false && "this framebuffer has already been created!");
		return false;
	}
	if (!desc.validate()) {
		ERROR("Invalid framebuffer configuration in descriptor.");
		return false;
	}
	checkGLError();
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &previousFramebufferBinding_);
	glGenFramebuffers(1, &framebufferId_);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebufferId_);

	if (desc.multisamples > 0) {
		fbTextureId_ = 0;
		glGenRenderbuffers(1, &fbRenderbufferId_);
		glBindRenderbuffer(GL_RENDERBUFFER, fbRenderbufferId_);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, desc.multisamples, desc.format, desc.width, desc.height);
		glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, fbRenderbufferId_);
	} else {
		fbRenderbufferId_ = 0;
		glGenTextures(1, &fbTextureId_);
		glBindTexture(GL_TEXTURE_2D, fbTextureId_);
		glTexImage2D(GL_TEXTURE_2D, 0, desc.format, desc.width, desc.height, 0, desc.textureChannels(), desc.textureDataType(), nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fbTextureId_, 0);
	}

	if (desc.requireDepthBuffer) {
		glGenRenderbuffers(1, &depthRenderbufferId_);
		glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbufferId_);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, desc.multisamples, GL_DEPTH24_STENCIL8, desc.width, desc.height);
		glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthRenderbufferId_);
	} else
		depthRenderbufferId_ = 0;

	bool result = !checkGLError("gltCreateFrameBuffer") && glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, previousFramebufferBinding_);

	created_ = result;

	return result;
}

// destroys the openGL resources associated with this framebuffer.
// This is automatically called on the destructor as well, but is provided as user callable as well.
void FrameBuffer::destroy() {
	if (!created_) {
		ERROR("Framebuffer::destroy(): Attempting to destroy already destroyed Framebuffer");
		assertDbg(false);
		return;
	}
	assertDbg(!active_ && "attempting to destroy a framebuffer which is currently active. Unbind it first");

	if (fbTextureId_ > 0)
		glDeleteTextures(1, &fbTextureId_);
	if (fbRenderbufferId_ > 0)
		glDeleteRenderbuffers(1, &fbRenderbufferId_);
	if (depthRenderbufferId_ > 0)
		glDeleteRenderbuffers(1, &depthRenderbufferId_);
	glDeleteRenderbuffers(1, &framebufferId_);

	depthRenderbufferId_ = 0;
	fbRenderbufferId_ = 0;
	fbTextureId_ = 0;
	framebufferId_ = 0;
	created_ = false;
}

// bind this framebuffer as the DRAW framebuffer
void FrameBuffer::bind() {
	assertDbg(created_ && "attempting to bind an invalid framebuffer (forgot to call create()?)");
	assertDbg(!active_ && "attempting to bind a framebuffer which is already bound.");

	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &previousFramebufferBinding_);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebufferId_);

	glGetBooleanv(GL_DEPTH_WRITEMASK, &previousDepthMask_);
	if (depthRenderbufferId_)
		glDepthMask(GL_TRUE);
	else
		glDepthMask(GL_FALSE);

	active_ = true;
}

// unbinds this framebuffer and restores the previously bound DRAW framebuffer
void FrameBuffer::unbind() {
	assertDbg(active_ && "attempting to unbind a framebuffer which is not bound.");

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, previousFramebufferBinding_);
	glDepthMask(previousDepthMask_);

	active_ = false;
}

// bind this framebuffer as the READ framebuffer
void FrameBuffer::bindRead() {
	assertDbg(created_ && "attempting to bind an invalid framebuffer (forgot to call create()?)");
	assertDbg(!activeRead_ && "attempting to bind a framebuffer which is already bound.");

	glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &previousFramebufferBindingRead_);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferId_);

	activeRead_ = true;
}

// unbinds this framebuffer and restores the previously bound READ framebuffer
void FrameBuffer::unbindRead() {
	assertDbg(activeRead_ && "attempting to unbind a framebuffer which is not bound.");

	glBindFramebuffer(GL_READ_FRAMEBUFFER, previousFramebufferBindingRead_);

	activeRead_ = false;
}
