#include "medals.h"
#include "box.h"

Medals::Medals(int w, int h) {
	_modal = true;
	add(0, 0, new Box("menu/background_box_dark.png", w, h));
}

bool Medals::onKey(const SDL_keysym sym) {
	if (Container::onKey(sym))
		return true;

	switch(sym.sym) {
	case SDLK_ESCAPE: 
		hide();
		return true;
	default: 
		return true;
	}
}