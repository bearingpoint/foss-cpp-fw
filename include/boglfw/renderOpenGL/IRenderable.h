/*
 * IRenderable.h
 *
 *  Created on: Nov 9, 2014
 *      Author: bog
 */

#ifndef RENDEROPENGL_IRENDERABLE_H_
#define RENDEROPENGL_IRENDERABLE_H_

class Viewport;

class IRenderable {
public:
	virtual ~IRenderable() {}

	// this signals the component to treat all following draw commands as a new batch, and render them
	// in a separate pass to achieve layering
	virtual void startBatch() = 0;

	// called once per frame before per-viewport rendering to allow preparation of render data
	virtual void setupFrameData() = 0;

	 // This is called on the object at each frame, for each active viewport, for each draw batch
	virtual void render(Viewport* pCrtViewport, unsigned batchId) = 0;

	 // Called once per frame, after viewports are finished, to clear queued data
	virtual void purgeRenderQueue() = 0;

	// called once when the renderer is destroyed to release all resources associated with this renderable.
	virtual void unload() = 0;

	virtual const char* getName() const = 0;
};


#endif /* RENDEROPENGL_IRENDERABLE_H_ */
