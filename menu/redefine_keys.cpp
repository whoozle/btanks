#include "redefine_keys.h"
#include "resource_manager.h"
#include "sdlx/surface.h"
#include "sdlx/font.h"

RedefineKeys::RedefineKeys() {
	_bg_table = ResourceManager->loadSurface("menu/keys_table.png");
	_font = ResourceManager->loadFont("medium", true);
	_background.init("menu/background_box.png", _bg_table->getWidth() + 32, _bg_table->getHeight() + 32);
	
	_actions.push_back("up");
	_actions.push_back("down");
	_actions.push_back("left");
	_actions.push_back("right");
	_actions.push_back("fire");
	_actions.push_back("alt-fire");
	_actions.push_back("disembark");
}

void RedefineKeys::render(sdlx::Surface &surface, const int x, const int y) {
	_background.render(surface, x, y);
	int dx = (_background.w - _bg_table->getWidth()) / 2, dy = (_background.h - _bg_table->getHeight()) / 2;
	surface.copyFrom(*_bg_table, x + dx, y + dy);
	
	int yp = y + dy + 50;
	for(size_t i = 0; i < _actions.size(); ++i) {
		_font->render(surface, x + 48, yp, _actions[i]);
		yp += 30;
	}
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
