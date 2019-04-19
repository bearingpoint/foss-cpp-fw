#ifndef BOGLFW_FRAMEBUFFER_H
#define BOGLFW_FRAMEBUFFER_H

#include <boglfw/utils/assert.h>

#include <GL/glew.h>

// Describes a framebuffer's parameters.
// depending on multisamples parameter, the behaviour is slightly different:
//		if [multisamples]==0 then the created framebuffer will get a texture attached as the color attachment;
//		if [multisamples] > 0 then the created framebuffer will get a renderbuffer attached as the color attachment;
// validate() validates the parameters.
struct FrameBufferDescriptor {
	unsigned width = 256;
	unsigned height = 256;
	int format = GL_RGB;
	unsigned multisamples = 0;
	bool requireDepthBuffer = false;

	bool validate() const;

	GLuint textureChannels() const;
	GLuint textureDataType() const;
};

// this object represents an OpenGL frame-buffer along with all its color (texture or renderbuffer) and depth (renderbuffer) attachments
class FrameBuffer {
public:
	FrameBuffer() = default;
	FrameBuffer(FrameBuffer const&) = delete;	// prevent copy
	FrameBuffer(FrameBuffer &&fb);				// allow move
	~FrameBuffer();

	// returns true if this represents a valid and ready to use framebuffer.
	bool valid() const { return created_; }
	// returns true if this framebuffer is currently bound as the DRAW framebuffer.
	bool isActive() const { return active_; }

	unsigned framebufferId() const { assertDbg(created_ && "this is an invalid framebuffer"); return framebufferId_; }
	unsigned fbTextureId() const { assertDbg(created_ && "this is an invalid framebuffer"); return fbTextureId_; }
	unsigned fbRenderbufferId() const { assertDbg(created_ && "this is an invalid framebuffer"); return fbRenderbufferId_; }
	unsigned depthRenderbufferId() const { assertDbg(created_ && "this is an invalid framebuffer"); return depthRenderbufferId_; }

	// tries to create the OpenGL framebuffer resources; returns true on success and makes this object a valid framebuffer,
	// or false on failure.
	bool create(FrameBufferDescriptor const& desc);

	// destroys the openGL resources associated with this framebuffer.
	// This is automatically called on the destructor as well, but is provided as user callable as well.
	void destroy();

	// bind this framebuffer as the DRAW framebuffer
	void bind();
	// unbinds this framebuffer and restores the previously bound DRAW framebuffer
	void unbind();

	// binds this framebuffer as the READ framebuffer
	void bindRead();
	// unbinds this framebuffer and restores the previously bound READ framebuffer
	void unbindRead();

private:
	unsigned framebufferId_ = 0;
	unsigned fbTextureId_ = 0;
	unsigned fbRenderbufferId_ = 0;
	unsigned depthRenderbufferId_ = 0;

	bool created_ = false;
	bool active_ = false;
	bool activeRead_ = false;
	int previousFramebufferBinding_ = 0;
	int previousFramebufferBindingRead_ = 0;
	GLboolean previousDepthMask_ = false;
};

#endif // BOGLFW_FRAMEBUFFER_H
