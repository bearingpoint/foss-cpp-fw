/*
 * Window.cpp
 *
 *  Created on: Mar 25, 2015
 *      Author: bog
 */

#include <boglfw/GUI/Window.h>
#include <boglfw/GUI/GuiTheme.h>
#include <boglfw/GUI/ICaptureManager.h>
#include <boglfw/renderOpenGL/Shape2D.h>
#include <boglfw/math/math3D.h>

Window::Window(glm::vec2 position, glm::vec2 size)
	: GuiContainerElement(position, size)
{
	setClientArea(glm::vec2(3, 20), glm::vec2(3, 3));
}

Window::~Window() {
}

void Window::draw(RenderContext const& ctx, glm::vec2 frameTranslation, glm::vec2 frameScale) {
	glm::ivec2 trans(frameTranslation);
	// draw frame
	glm::vec2 scaledSize = getSize();
	scaledSize.x *= frameScale.x;
	scaledSize.y *= frameScale.y;
	Shape2D::get()->drawRectangleFilled(vec3xy(trans), scaledSize, GuiTheme::getWindowColor());
	Shape2D::get()->drawRectangle(vec3xy(trans), scaledSize, GuiTheme::getWindowFrameColor());

	// draw client area frame:
	glm::vec2 clientOffset, clientSize;
	getClientArea(clientOffset, clientSize);
	clientSize.x *= frameScale.x;
	clientSize.y *= frameScale.y;
	Shape2D::get()->drawRectangleFilled(vec3xy(trans)+clientOffset,
			clientSize, GuiTheme::getClientColor());
	Shape2D::get()->drawRectangle(vec3xy(trans)+clientOffset,
			clientSize, GuiTheme::getClientFrameColor());

	// now draw contents:
	GuiContainerElement::draw(ctx, glm::vec2(trans.x, trans.y), frameScale);
}

/*void Window::mouseDown(MouseButtons button) {
	if (button == MouseButtons::Left) {
		downPosition_ = getLastMousePosition();
		if (getPointedElement() == nullptr)
			getCaptureManager()->setMouseCapture(this);
	}
	GuiContainerElement::mouseDown(button);
}

void Window::mouseUp(MouseButtons button) {
	if (button == MouseButtons::Left && getPointedElement() == nullptr)
		getCaptureManager()->setMouseCapture(nullptr);
	GuiContainerElement::mouseUp(button);
}

void Window::mouseMoved(glm::vec2 delta, glm::vec2 position) {
	if (isMousePressed(MouseButtons::Left) && getPointedElement() == nullptr)
		setPosition(getPosition() + delta);
	else
		GuiContainerElement::mouseMoved(delta, position);
}*/
