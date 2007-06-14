
/* Battle Tanks Game
 * Copyright (C) 2006-2007 Battle Tanks team
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
#include "start_server_menu.h"
#include "button.h"
#include "mrt/logger.h"
#include "menu.h"
#include "menu_config.h"
#include "map_picker.h"
#include "game.h"
#include "game_monitor.h"
#include "map_desc.h"
#include "player_manager.h"
#include "i18n.h"

StartServerMenu::StartServerMenu(MainMenu *parent, const int w, const int h) : _parent(parent)  {
	_back = new Button("big", I18n->get("menu", "back"));
	add(64, h - 96, _back);
	
	_start = new Button("big", I18n->get("menu", "start"));
	int bw, bh;
	_start->getSize(bw, bh);
	add(w - 64 - bw, h - 96, _start);

	add(0, 0, _map_picker = new MapPicker(w, h));
}

void StartServerMenu::start() {
	const MapDesc &map = _map_picker->getCurrentMap();
	if (map.slots < 1) {
		GameMonitor->displayMessage("menu", "no-slots-in-map", 1);
		return;
	}

	LOG_DEBUG(("start multiplayer server requested"));
	Game->clear();
	GameMonitor->loadMap(NULL, map.name);
		
	_map_picker->fillSlots();
	
	PlayerManager->startServer();
	MenuConfig->save();

	_parent->back();
	return;
}

void StartServerMenu::tick(const float dt) {
	Container::tick(dt);
	if (_back->changed()) {
		LOG_DEBUG(("[back] clicked"));
		_back->reset();
		_parent->back();
		MenuConfig->save();
	}
	if (_start->changed()) {
		_start->reset();
		start();
	}

}

bool StartServerMenu::onKey(const SDL_keysym sym) {
	if (Container::onKey(sym))
		return true;

	switch(sym.sym) {

	case SDLK_RETURN:
		start();
		return true;

	case SDLK_ESCAPE: 
		_parent->back();
		MenuConfig->save();
		return true;

	default: ;
	}
	return false;
}


StartServerMenu::~StartServerMenu() {}
