
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
#include <assert.h>
#include <algorithm>

#include "map_picker.h"
#include "map_details.h"
#include "player_picker.h"
#include "scroll_list.h"
#include "i18n.h"
#include "player_slot.h"
#include "player_manager.h"
#include "map_desc.h"
#include "upper_box.h"
#include "window.h"
#include "menu_config.h"
#include "finder.h"
#include "nickname.h"

#include "mrt/exception.h"
#include "mrt/directory.h"
#include "mrt/xml.h"
#include "mrt/random.h"

#include "config.h"

#include "tmx/map.h"
#include "mrt/scoped_ptr.h"
#include "mrt/base_file.h"
#include "game_monitor.h"
#include "mode_panel.h"

struct MapScanner : mrt::XMLParser {
	int slots;
	std::string object_restriction;
	GameType game_type;
	bool supports_ctf;

	MapScanner() : slots(0), game_type(GameTypeDeathMatch), supports_ctf(false) {}

	void scan(const std::string &name) {
		scoped_ptr<mrt::BaseFile> f(Finder->get_file(Finder->find("maps/" + name + ".tmx"), "rt"));

		parseFile(*f);
		LOG_DEBUG(("parser: slots: %d, object_restriction: '%s'", slots, object_restriction.c_str()));
	}
private: 
	virtual void start(const std::string &name, Attrs &attr) {
		if (name == "property") {
			const std::string &pname = attr["name"];
			const std::string &pvalue = attr["value"];
			if (pname.compare(0, 6, "spawn:") == 0) {
				++slots;
			} else if (pname == "config:multiplayer.restrict-start-vehicle" && pvalue.compare(0, 7, "string:") == 0) {
				object_restriction = pvalue.substr(7);
			} else if (pname == "config:multiplayer.game-type" && pvalue.compare(0, 7, "string:") == 0) {
				std::string type = pvalue.substr(7);
				if (type == "deathmatch") {
					game_type = GameTypeDeathMatch;
				} else if (type == "cooperative") {
					game_type = GameTypeCooperative;
				} else if (type == "racing") {
					game_type = GameTypeRacing;
				} else 
					throw_ex(("unsupported game type '%s'", type.c_str()));
			} else if (pname.compare(0, 11, "object:ctf-") == 0) {
				supports_ctf = true;
			}
		}
	}
	virtual void end(const std::string &name) {}
//	virtual void charData(const std::string &data);
};


void MapPicker::scan(const std::string &base) {
	std::vector<std::string> entries;
	Finder->enumerate(entries, base, "maps");
	for(size_t i = 0; i < entries.size(); ++i) {
		std::string map = entries[i];
		
		mrt::toLower(map);
		if (map.size() < 5 || map.compare(map.size() - 4, 4, ".tmx") != 0)
			continue;
		map = map.substr(0, map.size() - 4);
		if (GameMonitor->usedInCampaign(base, map))
			continue;
		LOG_DEBUG(("found map: %s", map.c_str()));
		MapScanner m;
		TRY {
			m.scan(map);
		} CATCH("scanning map", {});
		_maps.push_back(MapList::value_type(base, map, m.object_restriction, m.game_type, m.slots, m.supports_ctf));
	}	
}

void MapPicker::tick(const float dt) {
	if (_upper_box->changed() || _index != _list->get() || _list->changed()) {
		_index = _list->get();
		const MapDesc & map = _maps[_index];

		_list->reset();
		_upper_box->reset();
		_upper_box->update(map.game_type);

		Config->set("menu.default-mp-map", map.name);
		_details->set(map);
		_picker->set(map);
		_mode_panel->set(map);
	}
	Container::tick(dt);
}

#include "notepad.h"

MapPicker::MapPicker(const int w, const int h) : _index(0) {
	std::vector<std::string> path;
	Finder->getPath(path);
	for(size_t i = 0; i < path.size(); ++i) {
		scan(path[i]);
	}
	
	if (_maps.empty())
		throw_ex(("no maps found. sorry. install some maps/reinstall game."));
		
	std::sort(_maps.begin(), _maps.end());
	
	std::string map;

	Config->get("menu.default-mp-map", map, "lenin_square");
	for(_index = 0; _index < (int)_maps.size(); ++_index) {
		if (_maps[_index].name == map)
			break;
	}
	if (_index >= (int)_maps.size())
		_index = 0;
	LOG_DEBUG(("map index: %d", _index));
	
	_upper_box = new UpperBox(w, 80, true);
	int xdummy, ybase;
	_upper_box->getSize(xdummy, ybase);
	ybase += 4;
	
	Notepad * notepad = new Notepad(w, "medium");
	
	notepad->add("menu/modes", "deathmatch");
	notepad->add("menu/modes", "team-deathmatch");
	notepad->add("menu/modes", "cooperative");
	notepad->add("menu/modes", "capture-the-flag");

	add(0, ybase, notepad);
	{
		int w, h;
		notepad->getSize(w, h);
		ybase += h;
	}

	sdlx::Rect list_pos(0, ybase, (w - 64)/3, h - 256);
	_list = NULL;
	TRY {
		_list = new ScrollList("menu/background_box.png", "medium", list_pos.w, list_pos.h);
		for(MapList::const_iterator i = _maps.begin(); i != _maps.end(); ++i) {
			_list->append(i->name);
		}
		add(list_pos.x, list_pos.y, _list);
		_list->set(_index);
	} CATCH("MapPicker::ctor", {delete _list; throw; });

	sdlx::Rect map_pos(list_pos.w + 16, ybase, (w - 64) / 3, h - 256);

	_picker = NULL;
	TRY {
		_picker = new PlayerPicker(w - map_pos.x - map_pos.w - 16, h - 256);
		_picker->set(_maps[_index]);
		add(map_pos.x + map_pos.w + 16, ybase, _picker);
	} CATCH("PlayerPicker::ctor", {delete _picker; throw; });

	
	TRY {
		int cw, ch;
		_upper_box->getSize(cw, ch);
		add((w - cw) / 2, 0, _upper_box);
	} CATCH("StartServerMenu", {delete _upper_box; throw; });

	_details = NULL;	
	TRY {
		_details = new MapDetails(map_pos.w, map_pos.h);
		_details->set(_maps[_index]);
		add(map_pos.x, map_pos.y, _details);
	} CATCH("MapPicker::ctor", {delete _details; _details = NULL; throw; });

	int ydummy;
	_list->getSize(xdummy, ydummy);
	ybase += ydummy + 4;
	add(0, ybase, _mode_panel = new ModePanel(w));

}

void MapPicker::fillSlots() const {
	if (PlayerManager->getSlotsCount() < 1)
		return;

	bool split;
	Config->get("multiplayer.split-screen-mode", split, false);

	std::vector<SlotConfig> config;
	MenuConfig->fill(_maps[_index].name, _picker->getVariant(), config);
	int idx1 = -1, idx2 = -1;
	for(size_t i = 0; i < config.size(); ++i) {
		if (i >= (size_t)_maps[_index].slots)
			break;
		
		PlayerSlot &slot = PlayerManager->getSlot(i);
		std::string type = config[i].type;
		if (type.empty() || type == "?")
			continue;
		std::string object = config[i].vehicle;
		if (object == "?") {
			static const char *objects[] = {"launcher", "shilka", "tank"};
			object = objects[mrt::random(sizeof(objects) / sizeof(objects[0]))];
		}
		
		std::string animation;
		mrt::toLower(object);
		LOG_DEBUG(("before: %s:%s", object.c_str(), animation.c_str()));
		slot.getDefaultVehicle(object, animation);
		LOG_DEBUG(("after: %s:%s", object.c_str(), animation.c_str()));
		
		mrt::toLower(type);

		std::string cm = "ai";
		if (!split) {
			if (type == "player") {
				idx1 = i;
				Config->get("player.control-method", cm, "keys");
				Config->get("player.name-1", slot.name, Nickname::generate());
			} else 
				slot.name = Nickname::generate();
		} else {
			if (type == "player-1") {
				idx1 = i;
				Config->get("player.control-method-1", cm, "keys-1");		
				Config->get("player.name-1", slot.name, Nickname::generate());
			} else if (type == "player-2") {
				idx2 = i;
				Config->get("player.control-method-2", cm, "keys-2");
				Config->get("player.name-2", slot.name, Nickname::generate());
			} else 
				slot.name = Nickname::generate();
		}
		slot.createControlMethod(cm);
		slot.spawnPlayer(i, object, animation);
	}

	if (!split) {	
		PlayerManager->getSlot((idx1 == -1)?0:idx1).setViewport(Window->getSize());
	} else {
		v2<int> ts = Map->getTileSize();
		sdlx::Rect window_size = Window->getSize();
		int w = window_size.w / 2;

		sdlx::Rect vp1(window_size);
		sdlx::Rect vp2(window_size);
		vp1.w = w;

		vp2.x = w;
		vp2.w = w;
		LOG_DEBUG(("p1: %d %d %d %d", vp1.x, vp1.y, vp1.w, vp1.h));
		LOG_DEBUG(("p2: %d %d %d %d", vp2.x, vp2.y, vp2.w, vp2.h));
		PlayerManager->getSlot((idx1 == -1)?0:idx1).setViewport(vp1);
		PlayerManager->getSlot((idx2 == -1)?(idx1 != 1?1:0):idx2).setViewport(vp2); //avoid duplication of viewports
	}
	PlayerManager->validateViewports();
}
