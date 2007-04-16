#include "color.h"

sdlx::Color::Color(Uint8 r, Uint8 g, Uint8 b)
	 {
		SDL_Color::r = r;
		SDL_Color::g = g;
		SDL_Color::b = b;
	}
