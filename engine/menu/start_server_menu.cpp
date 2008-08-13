
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
#include "rt_config.h"
#include "config.h"
#include "box.h"

StartServerMenu::StartServerMenu(MainMenu *parent, const int w, const int h) : _parent(parent)  {
	_map_picker = new MapPicker(w, h);
	int y1, y2;
	_map_picker->getBaseSize(y1, y2);
	add(0, y1, new Box("menu/background_box.png", w, y2 - y1 - 16));
	
	int cw, ch;
	_map_picker->get_size(cw, ch);

	_back = new Button("big", I18n->get("menu", "back"));
	int bw, bh;
	_back->get_size(bw, bh);
	add(64, h - (h - ch) / 2 - bh / 2, _back);
	
	_start = new Button("big", I18n->get("menu", "start"));
	_start->get_size(bw, bh);
	add(w - 64 - bw, h - (h - ch) / 2 - bh / 2, _start);

	add(0, 0, _map_picker);
}

void StartServerMenu::start() {
	LOG_DEBUG(("starting the game"));
	
	const MapDesc &map = _map_picker->getCurrentMap();
	if (map.slots < 1) {
		GameMonitor->displayMessage("menu", "no-slots-in-map", 1);
		return;
	}
	int idx;
	Config->get("menu.default-game-mode", idx, 0);
	
	switch(idx) {
	case 3: //ctf
		if (!map.supports_ctf) 
			throw_ex(("start: map does not support ctf, but menu requested mode %d", idx));
		
		LOG_DEBUG(("starting map in CTF mode. good luck."));
		RTConfig->game_type = GameTypeCTF;
		RTConfig->teams = 2;
		break;

	case 1: { //team dethmatch 
		int teams;
		Config->get("multiplayer.teams", teams, 0);
		if (teams <= 0) 
			throw_ex(("start: requested team deathmatch, but teams == %d", teams));

		RTConfig->game_type = GameTypeTeamDeathMatch;
		RTConfig->teams = teams;
		} break;

	case 0: 
		if (map.game_type != GameTypeDeathMatch)
			throw_ex(("menu game type == deathmatch, map game type: %d", (int)map.game_type));
		RTConfig->game_type = map.game_type;
		break;

	case 2: 
		if (map.game_type != GameTypeCooperative)
			throw_ex(("menu game type == cooperative, map game type: %d", (int)map.game_type));
		RTConfig->game_type = map.game_type;
		break;
	
	default: 
		throw_ex(("unsupported game type %d", idx));
	}
	
	if (RTConfig->game_type != GameTypeCooperative && RTConfig->game_type != GameTypeRacing) {
		int tl; 
		Config->get("multiplayer.time-limit", tl, 0);
		RTConfig->time_limit = (float)tl;
	} else {
		RTConfig->time_limit = 0;
	}
	
	Game->clear();
	PlayerManager->start_server();
	GameMonitor->loadMap(NULL, map.name);
		
	_map_picker->fillSlots();
	
	MenuConfig->save();

	//_parent->back();
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

	case SDLK_KP_ENTER:
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
