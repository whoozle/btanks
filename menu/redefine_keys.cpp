#include "redefine_keys.h"
#include "resource_manager.h"
#include "sdlx/surface.h"
#include "sdlx/font.h"
#include "sdlx/rect.h"
#include "config.h"

static const std::string variants[] = {"keys", "keys-1", "keys-2"};


RedefineKeys::RedefineKeys() : _active_row(-1), _active_col(-1) {
	_bg_table = ResourceManager->loadSurface("menu/keys_table.png");
	_selection = ResourceManager->loadSurface("menu/keys_selection.png");
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
	
	reload();
//	Config->get("controls.keys.up", );
}

void RedefineKeys::reload() {
	_actions.clear();
	for(size_t i = 0; i < _labels.size(); ++i) {
		_actions.push_back(Actions::value_type(_labels[i], sdlx::Rect()));
		for(size_t j = 0; j < 3; ++j) {
			Config->get("player.controls." + variants[j] + "." + _labels[i], _keys[j][i], _keys[j][i]);
		}
	}
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

		if (_active_row == (int)i && _active_col != -1) {
			surface.copyFrom(*_selection, x + 173 + 110 * _active_col, yp - 6);
		}

		_font->render(surface, x + 36, yp, _actions[i].first);
		
		for(size_t j = 0; j < 3; ++j) {
			const char *cname = _keys[j][i] ? SDL_GetKeyName((SDLKey)_keys[j][i]): NULL;
			std::string name = (cname)?cname:"???";
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
		save();
		hide(true);
		return true;
		
	default: ;
	}
	
	if (_active_row == -1 || _active_col == -1) 
		return true;
	
	//LOG_DEBUG(("key: %s", SDL_GetKeyName(sym.sym)));
	int old_key = _keys[_active_col][_active_row];
	_keys[_active_col][_active_row] = sym.sym;
	//validation
	if (_active_col == 0) {
		for(int j = 0; j < 7; ++j) {
			if (j == _active_row)
				continue;
			if (_keys[0][j] == sym.sym)
				_keys[0][j] = old_key;
		}
	} else {
		for(int i = 1; i < 3; ++i) 
			for(int j = 0; j < 7; ++j) {
				if (i == _active_col && j == _active_row)
					continue;
				if (_keys[i][j] == sym.sym)
					_keys[i][j] = old_key;
			}
	}
	return true;
}

void RedefineKeys::save() {
	for(int i = 0; i < 3; ++i) 
		for(int j = 0; j < 7; ++j) {
			if (_keys[i][j] == 0)
				throw_ex(("invalid key code. (0)"));
	}

	for(size_t i = 0; i < _labels.size(); ++i) {
		for(size_t j = 0; j < 3; ++j) {
			Config->set("player.controls." + variants[j] + "." + _labels[i], _keys[j][i]);
		}
	}	
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
