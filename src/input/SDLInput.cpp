/*
 * GLFWInput.cpp
 *
 *  Created on: Oct 29, 2018
 *      Author: bog
 */

#include <boglfw/input/SDLInput.h>
#include <boglfw/input/InputEvent.h>
#include <SDL2/SDL_events.h>

SDL_Window* SDLInput::window_ = nullptr;
Event<void(InputEvent&)> SDLInput::onInputEvent;

void SDLInput::initialize(SDL_Window* window) {
	window_ = window;
}

bool SDLInput::checkInput() {
	SDL_Event sdl_ev;
	bool shouldQuit = false;
	while (SDL_PollEvent(&sdl_ev)) {
		if (sdl_ev.type == SDL_QUIT)
			shouldQuit = true;
		InputEvent ev;
		if (translate(sdl_ev, ev))
			onInputEvent.trigger(ev);
	}
	return !shouldQuit;
}

InputEvent::MOUSE_BUTTON SDLInput::translateMouseButton(int sdl_but) {
	return (InputEvent::MOUSE_BUTTON)((int)InputEvent::MB_LEFT + sdl_but - 1);
}

bool SDLInput::translate(SDL_Event const& e, InputEvent &out) {
	switch(e.type) {
	case SDL_KEYDOWN:
		new(&out) InputEvent(InputEvent::EV_KEY_DOWN, 0, 0, 0, 0, 0, InputEvent::MB_NONE, (int)e.key.keysym.sym, 0);
		return true;
	case SDL_KEYUP:
		new(&out) InputEvent(InputEvent::EV_KEY_UP, 0, 0, 0, 0, 0, InputEvent::MB_NONE, (int)e.key.keysym.sym, 0);
		return true;
	case SDL_TEXTINPUT:
		new(&out) InputEvent(InputEvent::EV_KEY_CHAR, 0, 0, 0, 0, 0, InputEvent::MB_NONE, 0, 'X'); // TODO
		return true;
	case SDL_MOUSEBUTTONDOWN:
		new(&out) InputEvent(InputEvent::EV_MOUSE_DOWN, e.button.x, e.button.y, 0, 0, 0, translateMouseButton(e.button.button), 0, 0);
		return true;
	case SDL_MOUSEBUTTONUP:
		new(&out) InputEvent(InputEvent::EV_MOUSE_UP, e.button.x, e.button.y, 0, 0, 0, translateMouseButton(e.button.button), 0, 0);
		return true;
	case SDL_MOUSEMOTION:
		new(&out) InputEvent(InputEvent::EV_MOUSE_MOVED, e.motion.x, e.motion.y, e.motion.xrel, e.motion.yrel, 0, InputEvent::MB_NONE, 0, 0);
		return true;
	case SDL_MOUSEWHEEL:
		new(&out) InputEvent(InputEvent::EV_MOUSE_SCROLL, e.motion.x, e.motion.y, 0, 0, e.wheel.y, InputEvent::MB_NONE, 0, 0);
		return true;
	default:
		return false;
	}
}
