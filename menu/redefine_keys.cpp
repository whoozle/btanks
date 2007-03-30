#include "redefine_keys.h"
#include "resource_manager.h"
#include "sdlx/surface.h"
#include "sdlx/font.h"
#include "sdlx/rect.h"

RedefineKeys::RedefineKeys() : _active_row(-1), _active_col(-1) {
	_bg_table = ResourceManager->loadSurface("menu/keys_table.png");
	_font = ResourceManager->loadFont("medium", true);
	_background.init("menu/background_box_dark.png", "menu/highlight_medium.png", _bg_table->getWidth() + 32, _bg_table->getHeight() + 32);
	
	_actions.push_back(Actions::value_type("up", sdlx::Rect()));
	_actions.push_back(Actions::value_type("down", sdlx::Rect()));
	_actions.push_back(Actions::value_type("left", sdlx::Rect()));
	_actions.push_back(Actions::value_type("right", sdlx::Rect()));
	_actions.push_back(Actions::value_type("fire", sdlx::Rect()));
	_actions.push_back(Actions::value_type("alt-fire", sdlx::Rect()));
	_actions.push_back(Actions::value_type("disembark", sdlx::Rect()));
}

void RedefineKeys::render(sdlx::Surface &surface, const int x, const int y) {
	_background.render(surface, x, y);
	int dx = (_background.w - _bg_table->getWidth()) / 2, dy = (_background.h - _bg_table->getHeight()) / 2;
	surface.copyFrom(*_bg_table, x + dx, y + dy);
	
	int yp = y + dy + 50;
	for(size_t i = 0; i < _actions.size(); ++i) {
		sdlx::Rect &rect = _actions[i].second;
		rect.x = 0;
		rect.y = yp - 15 - y;
		rect.h = _font->getHeight() + 30;
		rect.w = _background.w;
		
		if (_active_row == (int)i) {
			_background.renderHL(surface, x, yp + _font->getHeight() / 2 + 1);
		}
		_font->render(surface, x + 36, yp, _actions[i].first);
		yp += 30;
	}
}

void RedefineKeys::getSize(int &w, int &h) const {
	w = _background.w;
	h = _background.h;
}

bool RedefineKeys::onKey(const SDL_keysym sym) {
	switch(sym.sym) {

	case SDLK_ESCAPE: 
		hide(true);
		return true;
		
	default: ;
	}
	
	if (_active_row == -1 || _active_col == -1) 
		return true;
	
	LOG_DEBUG(("key: %s", SDL_GetKeyName(sym.sym)));
		
	
	return true;
}

bool RedefineKeys::onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel) {
	_active_col = _active_row = -1;
	int dx = (_background.w - _bg_table->getWidth()) / 2;
	for(size_t i = 0; i < _actions.size(); ++i) {
		const sdlx::Rect &rect = _actions[i].second;
		if (rect.in(x, y)) 
			_active_row = i;
		int col = (x - dx - 148);
		if (col >= 0) {
			col /=110;
			if (col >= 0 && col < 3)
				_active_col = col;
			//LOG_DEBUG(("col %d", _active_col));
		}
	}
	return true;
}
