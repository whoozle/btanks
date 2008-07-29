
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
#include "join_server_menu.h"
#include "button.h"
#include "mrt/logger.h"
#include "menu.h"
#include "menu_config.h"
#include "host_list.h"
//#include "map_details.h"
#include "prompt.h"
#include "text_control.h"
#include "player_manager.h"
#include "game.h"
#include "game_monitor.h"
#include "chooser.h"
#include "config.h"
#include "i18n.h"
#include "upper_box.h"
#include "net/scanner.h"
#include "host_item.h"

JoinServerMenu::JoinServerMenu(MainMenu *parent, const int w, const int h) : ping_timer(true), _parent(parent), _scanner(NULL) {
	ping_timer.set(60, false);
	
	_back = new Button("big", I18n->get("menu", "back"));
	_add = new Button("medium_dark",  I18n->get("menu", "add"));
	_del = new Button("medium_dark",  I18n->get("menu", "delete"));
	_scan = new Button("big", I18n->get("menu", "scan"));
	_join = new Button("big", I18n->get("menu", "join"));
	_upper_box = new UpperBox(w - 48, 80, false);
	_add_dialog = new Prompt(w / 2, 96, new HostTextControl("medium"));

	const int host_list_w = w - 128;

	int bw, bh, xp = 48;

	_add->get_size(bw, bh);
	add(16, h - 80 - bh, _add);

	_del->get_size(bw, bh);
	add(xp + host_list_w - 32 - bw, h - 80 - bh, _del);

	_back->get_size(bw, bh);
	add(xp, h - 16 - bh, _back);
	xp += 16 + bw;
	
	_scan->get_size(bw, bh);
	add(xp, h - 16 - bh, _scan);
	
	_join->get_size(bw, bh);
	add(w - 64 - bw, h - 16 - bh, _join);

	sdlx::Rect list_pos(16, 128, host_list_w, h - 256);

	_hosts = new HostList("multiplayer.recent-hosts", list_pos.w, list_pos.h);
	add(list_pos.x, list_pos.y, _hosts);
	
	_upper_box->get_size(bw, bh);
	add((w - bw) / 2 - 8, 32, _upper_box);

	sdlx::Rect map_pos(list_pos.x + list_pos.w + 16, 128, (w - 64) / 3, h - 256);

	//_details = new MapDetails(map_pos.w, map_pos.h);
	//add(map_pos.x, map_pos.y, _details);	
	
	_add_dialog->get_size(bw, bh);
	add(w / 3, (h - bh) / 2, _add_dialog);
	_add_dialog->hide();
	
	//client vehicle stub.
	std::vector<std::string> options;

	options.clear();
	options.push_back("?");
	options.push_back("launcher");
	options.push_back("shilka");
	options.push_back("tank");
	options.push_back("machinegunner");
	options.push_back("civilian");
	options.push_back("mortar");
		
	_vehicle = new Chooser("medium", options, "menu/vehicles.png", true);
	_vehicle2 = new Chooser("medium", options, "menu/vehicles.png", true);

	_vehicle->disable(0);
	_vehicle2->disable(0);

	for(int i = 4; i < _vehicle->size(); ++i) {
		_vehicle->disable(i);
		_vehicle2->disable(i);
	}

	TRY {
		std::string def_v;

		Config->get("menu.default-vehicle-1", def_v, "tank");
		_vehicle->set(def_v);

		Config->get("menu.default-vehicle-2", def_v, "tank");
		_vehicle2->set(def_v);
	} CATCH("_vehicle->set()", {})

	_vehicle->get_size(bw, bh);
		
	add(list_pos.w + 32, list_pos.y + 16, _vehicle);
	add(list_pos.w + 32, list_pos.y + bh + 32, _vehicle2);
}

void JoinServerMenu::join() {
	LOG_DEBUG(("join()"));
	if (_hosts->empty()) {
		LOG_DEBUG(("please add at least one host in list."));
		return;
	}
	
	HostItem *item = dynamic_cast<HostItem *>(_hosts->getItem(_hosts->get()));
	if (item == NULL)
		return;
		
	mrt::Socket::addr addr = item->addr;
	if (addr.ip == 0) {
		addr.getAddrByName(item->name);
		if (addr.ip == 0) {
			LOG_WARN(("ip undefined even after resolving :("));
			return;
		}
	}
		
	_hosts->promote();

	Config->set("menu.default-vehicle-1", _vehicle->getValue());
	
	bool split;
	Config->get("multiplayer.split-screen-mode", split, false);
		
	Game->clear();
	PlayerManager->start_client(addr, split?2:1);
}

void JoinServerMenu::activate(bool active) {
	if (active && _scanner == NULL) {
		_scanner = new Scanner;				
		_scanner->scan();
		ping();
	}
}


void JoinServerMenu::tick(const float dt) {
	Container::tick(dt);

	bool split;
	Config->get("multiplayer.split-screen-mode", split, false);

	if (split && _vehicle2->hidden())
		_vehicle2->hide(false);
	if (!split && !_vehicle2->hidden())
		_vehicle2->hide();

	if (_vehicle->changed()) {
		_vehicle->reset();
		Config->set("menu.default-vehicle-1", _vehicle->getValue());
	}

	if (_vehicle2->changed()) {
		_vehicle2->reset();
		Config->set("menu.default-vehicle-2", _vehicle2->getValue());
	}
	
	if (_back->changed()) {
		LOG_DEBUG(("[back] clicked"));
		MenuConfig->save();
		_back->reset();
		_parent->back();
	}
	if (_add->changed()) {
		_add->reset();
		_add_dialog->hide(false);
	}
	
	if (_del->changed()) {
		_del->reset();
		_hosts->remove(_hosts->get());
	}
	
	if (_add_dialog->changed()) {
		_add_dialog->reset();
		_add_dialog->hide();
		std::string host = _add_dialog->get();
		if (!host.empty()) {
			_hosts->append(host);
			ping();
		}
		
		_add_dialog->set(std::string());
	}


	if (_scan->changed()) {
		_scan->reset();
		ping_timer.reset();
		if (_scanner == NULL)
			_scanner = new Scanner;

		_scanner->scan();
		ping(); //ping creates scanner	
	}

	if (ping_timer.tick(dt)) {
		ping_timer.reset();
		ping();
	}
	
	if (_join->changed()) {
		_join->reset();
		join();
	}

	if (_scanner == NULL) {
		_scanner = new Scanner;
		_scanner->scan();
		ping(); 
	}

	if (_scanner != NULL && _scanner->changed()) {
		_scanner->reset();
		Scanner::HostMap hosts;
		_scanner->get(hosts);
		
		for(int i = 0; i < _hosts->size(); ++i) {
			HostItem * host = dynamic_cast<HostItem*>(_hosts->getItem(i));
			if (host == NULL) 
				continue;
			
			Scanner::HostMap::iterator h = hosts.find(host->addr);
			if (h != hosts.end()) {
				const Scanner::Host &src = h->second;
				host->name = src.name;
				host->ping = src.ping;
				host->players = src.players;
				host->slots = src.slots;
				host->map = src.map;
				host->game_type = src.game_type;
				host->update();
				hosts.erase(h);
			} 
		}
		for(Scanner::HostMap::iterator h = hosts.begin(); h != hosts.end(); ++h) {
			const Scanner::Host &src = h->second;
			HostItem *item = new HostItem;
			item->addr = h->first;
			item->name = src.name;
			item->map = src.map;
			item->ping = src.ping;
			item->players = src.players;
			item->slots = src.slots;
			_hosts->append(item);
		}
	
		std::set<mrt::Socket::addr> dup_ips;
		for(int i = 0; i < _hosts->size(); ) {
			HostItem * host = dynamic_cast<HostItem*>(_hosts->getItem(i));
			if (host == NULL) 
				continue;

			if (dup_ips.find(host->addr) != dup_ips.end()) {
				_hosts->remove(i);
				continue;
			}			
			
			dup_ips.insert(host->addr);
			++i;
		}
		update();
	}
	
	if (_hosts->changed()) {
		_hosts->reset();
		update();
	}
}

void JoinServerMenu::update() {
	if (_hosts->empty())
		return;
	
	//const HostItem * host = dynamic_cast<const HostItem*>(_hosts->getItem(_hosts->get()));
	//if (host == NULL) 
	//	return;
	//const std::string & map = host->map;
	//LOG_DEBUG(("showing map: %s", map.c_str()));
}

void JoinServerMenu::ping() {
	LOG_DEBUG(("ping()"));
	if (_scanner == NULL)
		_scanner = new Scanner;

	for(int i = 0; i < _hosts->size(); ++i) {
		HostItem * host = dynamic_cast<HostItem*>(_hosts->getItem(i));
		if (host == NULL) 
			continue;
		_scanner->add(host->addr, host->name);
		GET_CONFIG_VALUE("engine.rolling-slash-timeout", float, rst, 5.0f);
		host->start(rst);
	}	
}

bool JoinServerMenu::onKey(const SDL_keysym sym) {
	if (Container::onKey(sym))
		return true;
	
	switch(sym.sym) {

	case SDLK_KP_ENTER:
	case SDLK_RETURN:
		join();
		return true;
	
	case SDLK_a: 
		_add_dialog->hide(false);
		return true;
	
	case SDLK_ESCAPE: 
		MenuConfig->save();
		_parent->back();
		return true;
	default: ;
	}
	return false;
}

JoinServerMenu::~JoinServerMenu() {
	if (_scanner != NULL) {
		delete _scanner;
		_scanner = NULL;
	}
}
