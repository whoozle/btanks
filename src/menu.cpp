
/* Battle Tanks Game
 * Copyright (C) 2006 Battle Tanks team
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

#include "menu.h"
#include "menuitem.h"
#include "game.h"

#include "sdlx/surface.h"
#include "sdlx/ttf.h"
#include "sdlx/color.h"
#include "mrt/logger.h"
#include "mrt/exception.h"
#include "config.h"

void MainMenu::init(const int w, const int h) {
	deinit();

	_screen_w = w;
	_screen_h = h;
	_active = true;
	
	LOG_DEBUG(("loading font..."));
	GET_CONFIG_VALUE("engine.data-directory", std::string, data_dir, "data");
	_font.open(data_dir + "/font/Verdana.ttf", 18);

	LOG_DEBUG(("loading background..."));
	_background.loadImage(data_dir + "/tiles/menu_background.png");
	
	LOG_DEBUG(("creating menu..."));
	_active_item = 0;
	_active_menu.clear();
	
	_items[""].push_back(new MenuItem(_font, "start-game", "submenu", "START GAME"));
	_items[""].push_back(new MenuItem(_font, "multiplayer", "submenu", "MULTIPLAYER"));
	_items[""].push_back(new MenuItem(_font, "options", "submenu", "OPTIONS"));
	_items[""].push_back(new MenuItem(_font, "credits", "command", "CREDITS"));
	_items[""].push_back(new MenuItem(_font, "quit", "command", "QUIT"));
	
	_items["start-game"].push_back(new MenuItem(_font, "start:tank", "command", "USE TANK"));
	_items["start-game"].push_back(new MenuItem(_font, "start:launcher", "command", "USE LAUNCHER"));
	_items["start-game"].push_back(new MenuItem(_font, "start:shilka", "command", "USE SHILKA"));
	_items["start-game"].push_back(new MenuItem(_font, "back", "back", "BACK"));

	//_items["options"].push_back(new MenuItem(_font, "player1", "iterable", "PLAYER 1 CONTROL: AI"));
	
	_items["options"].push_back(new MenuItem(_font, "back", "back", "BACK"));

	_items["multiplayer"].push_back(new MenuItem(_font, "m-start", "command", "START NEW GAME"));
	_items["multiplayer"].push_back(new MenuItem(_font, "multiplayer-join", "submenu", "JOIN GAME"));
	_items["multiplayer"].push_back(new MenuItem(_font, "s-start", "command", "SPLIT SCREEN GAME"));
	_items["multiplayer"].push_back(new MenuItem(_font, "back", "back", "BACK"));

	_items["multiplayer-join"].push_back(new MenuItem(_font, "m-join", "command", "JOIN GAME"));
	_items["multiplayer-join"].push_back(new MenuItem(_font, "address", "text", "LOCALHOST"));
	_items["multiplayer-join"].push_back(new MenuItem(_font, "port", "text", "9876"));
	_items["multiplayer-join"].push_back(new MenuItem(_font, "back", "back", "BACK"));

	recalculateSizes();

	Game->key_signal.connect(sigc::mem_fun(this, &MainMenu::onKey));
}

void MainMenu::recalculateSizes() {
	_menu_size.w = _menu_size.h = 0;
	for(ItemList::const_iterator i = _items[_active_menu].begin(); i != _items[_active_menu].end(); ++i) {
		int w, h;
		(*i)->getSize(w, h);
		if (w > _menu_size.w) 
			_menu_size.w = w;
		_menu_size.h += h + 10;
	}
	_menu_size.x = (_screen_w - _menu_size.w) / 2;
	_menu_size.y = (_screen_h - _menu_size.h) / 2;
}

void MainMenu::deinit() {
	for(MenuMap::iterator m = _items.begin(); m != _items.end(); ++m) {
		for(ItemList::iterator i = m->second.begin(); i != m->second.end(); ++i) {
			delete *i;
			*i = NULL;
		}
	}
	_items.clear();
	_menu_path.clear();
	_active_menu.clear();
	_active_item = 0;
}

MainMenu::~MainMenu() { deinit(); }


void MainMenu::onKey(const Uint8 type, const SDL_keysym sym) {
	if (!_active)
		return;
	if (type != SDL_KEYDOWN)
		return;
	switch(sym.sym) {
		case SDLK_UP:
			if (_active_item == 0) 
				_active_item = _items[_active_menu].size() - 1;
			else --_active_item;
			break;

		case SDLK_DOWN:
			if (_active_item == _items[_active_menu].size() - 1) 
				_active_item = 0;
			else ++_active_item;
			break;

		case SDLK_RETURN: {
				MenuItem * item = _items[_active_menu][_active_item];
				const std::string &name = item->name;
				if (item->type == "submenu") {
					LOG_DEBUG(("entering submenu '%s'", name.c_str()));
					_menu_path.push_front(MenuID(_active_item, _active_menu));
					_active_menu = name;
					_active_item = 0;
					recalculateSizes();
				} else if (item->type == "back") {
					if (!back()) 
						throw_ex(("cannot do 'back' command from top-level menu"));
				} else if (item->type == "command") {
					LOG_DEBUG(("command: %s", name.c_str()));
					menu_signal.emit(name);
				} else if (item->type == "iterable") {
					item->onClick();
					recalculateSizes();
				} else throw_ex(("unknown menu item type: %s", item->type.c_str()));
			}
			break;
		default: 
			break;
	}
	//LOG_DEBUG(("active item = %u", _active_item));
}


void MainMenu::render(sdlx::Surface &dst) {
	if (!_active)
		return;
		
	dst.copyFrom(_background, (dst.getWidth() - _background.getWidth()) / 2, (dst.getHeight() - _background.getHeight()) / 2);
	
	int x = _menu_size.x;
	int y = _menu_size.y;
	
	const ItemList & items = _items[_active_menu];
	size_t n = items.size();
	for(size_t i = 0; i < n ;++i) {
		int w,h;
		items[i]->getSize(w, h);
		items[i]->render(dst, x + (_menu_size.w - w) / 2, y, i == _active_item);
		y += h + 10;
	}
}

void MainMenu::setActive(const bool a) {
	_active = a;
}

void MainMenu::reset() {
	_menu_path.clear();
	_active_menu.clear();
	_active_item = 0;
}

const bool MainMenu::back() {
	if (_menu_path.size() == 0) 
		return false;
	_active_item = _menu_path.front().first;
	_active_menu = _menu_path.front().second;
	
	_menu_path.pop_front();
	recalculateSizes();
	return true;
}
