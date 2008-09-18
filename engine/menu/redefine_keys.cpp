
/* Battle Tanks Game
 * Copyright (C) 2006-2008 Battle Tanks team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
*/
#include "redefine_keys.h"
#include "resource_manager.h"
#include "sdlx/surface.h"
#include "sdlx/font.h"
#include "sdlx/rect.h"
#include "config.h"
#include "i18n.h"
#include "button.h"

static const std::string variants[] = {"keys", "keys-1", "keys-2"};

void RedefineKeys::initDefaults() {
#include "controls/default_keys.cpp"

	memcpy(_keys, keys, sizeof(_keys));
}


RedefineKeys::RedefineKeys() : _active_row(-1), _active_col(-1) {
	_bg_table = ResourceManager->loadSurface("menu/keys_table.png");
	_selection = ResourceManager->loadSurface("menu/keys_selection.png");
	_font = ResourceManager->loadFont("medium", true);
	_small_font = ResourceManager->loadFont("small", true);

	_background.init("menu/background_box_dark.png", _bg_table->get_width() + 96, _bg_table->get_height() + 140, 24);
	
	initDefaults();
	
	_labels.push_back("up");
	_labels.push_back("down");
	_labels.push_back("left");
	_labels.push_back("right");
	_labels.push_back("fire");
	_labels.push_back("alt-fire");
	_labels.push_back("disembark");
	_labels.push_back("hint-ctrl");
	
	_b_ok = new Button("medium_dark", I18n->get("menu", "ok"));
	_b_default = new Button("medium_dark", I18n->get("menu", "default-keys"));
	
	int w, h;
	int mx, my;
	_background.getMargins(mx, my);
	
	_b_ok->get_size(w, h);
	int ym = 32;
	int yp = _background.h - h - ym;
	add (_background.w - mx - w, yp, _b_ok);

	_b_default->get_size(w, h);
	yp = _background.h - h - ym;
	add (_background.w / 2 - w / 2, yp, _b_default);
	
	reload();
	_modal = true;
}

void RedefineKeys::tick(const float dt) {
	Container::tick(dt);

	if (_b_ok->changed()) {
		_b_ok->reset();
		save();
		hide();
	}
	
	if (_b_default->changed()) {
		_b_default->reset();
		initDefaults();
	}
}


void RedefineKeys::reload() {
	_actions.clear();
	for(size_t i = 0; i < _labels.size(); ++i) {
		_actions.push_back(Actions::value_type(I18n->get("menu", _labels[i]), sdlx::Rect()));
		for(size_t j = 0; j < 3; ++j) {
			Config->get("player.controls." + variants[j] + "." + _labels[i], _keys[j][i], _keys[j][i]);
		}
	}
}

void RedefineKeys::render(sdlx::Surface &surface, const int x, const int y) const {
	_background.render(surface, x, y);
	int dx = (_background.w - _bg_table->get_width()) / 2, dy = (_background.h - _bg_table->get_height()) / 2;
	surface.blit(*_bg_table, x + dx, y + dy);
	
	int yp = y + dy + 50;
	for(size_t i = 0; i < _actions.size(); ++i) {
		sdlx::Rect &rect = _actions[i].second;
		rect.x = 0;
		rect.y = yp - 15 - y;
		rect.h = _font->get_height() + 30;
		rect.w = _background.w;
		
		if (_active_row == (int)i) {
			_background.renderHL(surface, x, yp + _font->get_height() / 2 + 1);
		}

		if (_active_row == (int)i && _active_col != -1) {
			surface.blit(*_selection, x + 205 + 110 * _active_col, yp - 6);
		}

		_font->render(surface, x + 66, yp, _actions[i].first);
		
		for(size_t j = 0; j < 3; ++j) {
			const char *cname = _keys[j][i] ? SDL_GetKeyName((SDLKey)_keys[j][i]): NULL;
			std::string name = (cname)?cname:"???";
			_small_font->render(surface, x + dx + 155 + 110 * j, yp + (_font->get_height() - _small_font->get_height()) / 2, name);
		}
		
		yp += 30;
	}
	Container::render(surface, x, y);
}

void RedefineKeys::get_size(int &w, int &h) const {
	Container::get_size(w, h);
	
	if (_background.w > w)
		w = _background.w;
	
	if (_background.h > h)
		h = _background.h;
}

bool RedefineKeys::onKey(const SDL_keysym sym) {
	switch(sym.sym) {

	case SDLK_RETURN:
	case SDLK_ESCAPE: 
		hide(true);
		return true;
	
	case SDLK_TAB:
	case SDLK_F12:
	case SDLK_m:
	case SDLK_KP_ENTER:
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

bool RedefineKeys::onMouse(const int button, const bool pressed, const int x, const int y) {
	Container::onMouse(button, pressed, x, y);
	return true;
}


bool RedefineKeys::onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel) {
	_active_col = _active_row = -1;
	int dx = (_background.w - _bg_table->get_width()) / 2;
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
