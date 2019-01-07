/*
 * GLFWInput.h
 *
 *  Created on: Oct 29, 2018
 *      Author: bog
 */

#ifndef __SDLINPUT_H__
#define __SDLINPUT_H__

#ifdef WITH_SDL

#include <boglfw/input/InputEvent.h>
#include <boglfw/utils/Event.h>

union SDL_Event;
struct SDL_Window;

class SDLInput {
public:
	static void initialize(SDL_Window* window);

	// returns true if application should continue, and false if it should shut down (user closed window)
	static bool checkInput();

	static Event<void(InputEvent&)> onInputEvent;
private:
	static SDL_Window *window_;
	static bool translate(SDL_Event const& ev, InputEvent &out);
	static InputEvent::MOUSE_BUTTON translateMouseButton(int sdl_but);
};

#else
#error "SDL support not enabled in this build, use -DWITH_SDL to enable it."
#endif // WITH_SDL

#endif // __SDLINPUT_H__