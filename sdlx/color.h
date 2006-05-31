#ifndef __SDLX_COLOR_H__
#define __SDLX_COLOR_H__

#include <SDL/SDL.h>

namespace sdlx {
class Color : public SDL_Color {
public:
	Color(Uint8 r, Uint8 g, Uint8 b) {
		SDL_Color::r = r;
		SDL_Color::g = g;
		SDL_Color::b = b;
	}
};
}


#endif

