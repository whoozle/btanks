#include "redefine_keys.h"
#include "resource_manager.h"
#include "sdlx/surface.h"
#include "sdlx/font.h"
#include "sdlx/rect.h"
#include "config.h"

RedefineKeys::RedefineKeys() : _active_row(-1), _active_col(-1) {
	_bg_table = ResourceManager->loadSurface("menu/keys_table.png");
	_font = ResourceManager->loadFont("medium", true);
	_small_font = ResourceManager->loadFont("small", true);
	_background.init("menu/background_box_dark.png", "menu/highlight_medium.png", _bg_table->getWidth() + 32, _bg_table->getHeight() + 32);
	
	static int keys[3][7] = {
		{SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_LCTRL, SDLK_LALT, SDLK_F1},
		{SDLK_r, SDLK_f, SDLK_d, SDLK_g, SDLK_q, SDLK_a, SDLK_F1},
		{SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_RCTRL, SDLK_RSHIFT, SDLK_F2},
	};
	memcpy(_keys, keys, sizeof(_keys));
	
	_labels.push_back("up");
	_labels.push_back("down");
	_labels.push_back("right");
	_labels.push_back("left");
	_labels.push_back("fire");
	_labels.push_back("alt-fire");
	_labels.push_back("disembark");
	
	static const std::string variants[] = {"keys", "keys-1", "keys-2"};

	for(size_t i = 0; i < _labels.size(); ++i) {
		_actions.push_back(Actions::value_type(_labels[i], sdlx::Rect()));
		for(size_t j = 0; j < 3; ++j) {
			Config->get("player-controls." + variants[j] + "." + _labels[i], _keys[j][i], _keys[j][i]);
		}
	}
	
//	Config->get("controls.keys.up", );
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
		
		for(size_t j = 0; j < 3; ++j) {
			const char *cname = SDL_GetKeyName((SDLKey)_keys[j][i]);
			std::string name = (cname)?cname:"?";
			_small_font->render(surface, x + dx + 155 + 110 * j, yp + (_font->getHeight() - _small_font->getHeight()) / 2, name);
		}
		
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
