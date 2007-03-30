#include "redefine_keys.h"
#include "resource_manager.h"
#include "sdlx/surface.h"

RedefineKeys::RedefineKeys() {
	_bg_table = ResourceManager->loadSurface("menu/keys_table.png");
	_background.init("menu/background_box.png", _bg_table->getWidth() + 32, _bg_table->getHeight() + 32);
}

void RedefineKeys::render(sdlx::Surface &surface, const int x, const int y) {
	_background.render(surface, x, y);
	surface.copyFrom(*_bg_table, x + (_background.w - _bg_table->getWidth()) / 2, x + (_background.h - _bg_table->getHeight()) / 2);
}

void RedefineKeys::getSize(int &w, int &h) const {
	w = _background.w;
	h = _background.h;
}

bool RedefineKeys::onKey(const SDL_keysym sym) {
//	if (Container::onKey(sym)) 
//		return true;
	switch(sym.sym) {

	case SDLK_ESCAPE: 
		hide(true);
	default: ;

	}
	
	return true;
}
