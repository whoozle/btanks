
/* Battle Tanks Game
 * Copyright (C) 2006-2009 Battle Tanks team
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
#include "menu/notepad.h"

struct MapScanner : mrt::XMLParser {
	int slots;
	std::string object_restriction;
	GameType game_type;
	bool supports_ctf;

	MapScanner() : slots(0), game_type(GameTypeDeathMatch), supports_ctf(false) {}

	void scan(const std::string &name) {
		scoped_ptr<mrt::BaseFile> f(Finder->get_file(Finder->find("maps/" + name + ".tmx"), "rt"));

		parse_file(*f);
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
				game_type = IRTConfig::parse_game_type(pvalue.substr(7));
			} else if (pname.compare(0, 11, "object:ctf-") == 0) {
				supports_ctf = true;
			}
		}
	}
	virtual void end(const std::string &name) {}
//	virtual void cdata(const std::string &data);
};

const MapDesc &MapPicker::getCurrentMap() const { 
	std::map<const int, int>::const_iterator i = map_indexes.find(_index);
	if (i == map_indexes.end())
		throw_ex(("getCurrentMap called before initialization"));
	int idx = i->second;
	if (idx < 0 || idx >= (int) _maps.size())
		throw_ex(("index %d is out of range", idx));
	return _maps[idx];
}


void MapPicker::scan(const std::string &base) {
	std::vector<std::string> entries;
	Finder->enumerate(entries, base, "maps");
	for(size_t i = 0; i < entries.size(); ++i) {
		std::string map = entries[i];
		
		mrt::to_lower(map);
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
		int  real = map_indexes[_index];
		assert(real >= 0 && real < (int)_maps.size());
		const MapDesc & map = _maps[real];

		_list->reset();
		_upper_box->reset();
		_upper_box->update(map.game_type);

		Config->set(mrt::format_string("menu.mode-%d.default-mp-map", notepad->get()), map.name);
		_details->set(map);
		_picker->set(map);
		_mode_panel->set(map, notepad->get());
	}
	if (notepad->changed()) {
		notepad->reset();
		int idx = notepad->get();
		Config->set("menu.default-game-mode", idx);
		reload();
	}
	Container::tick(dt);
}

static inline bool map_visible(const int mode, const MapDesc &map) {
	return (mode == 2 && map.game_type == GameTypeCooperative) ||
		   (mode == 3 && map.supports_ctf) || 
		   (mode < 2 && map.game_type == GameTypeDeathMatch);
}

void MapPicker::reload() {
	int mode = notepad->get();
	MenuConfig->load(mode);
	
	std::string map, default_map = (mode == 2)?"baykonur": "curfew";
	Config->get(mrt::format_string("menu.mode-%d.default-mp-map", mode), map, default_map);

	_index = 0;

	_list->clear();
	map_indexes.clear();
	
	for(size_t i = 0; i < _maps.size(); ++i) {
		if (map_visible(mode, _maps[i])) {
			int list_idx = _list->size();
			if (_maps[i].name == map)
				_index = list_idx;
			map_indexes[list_idx] = i;
			_list->append(_maps[i].name);
		}
	}
	LOG_DEBUG(("map index: %d, mode: %d", _index, mode));
	_list->set(_index);
}

MapPicker::MapPicker(const int w, const int h) : _index(0) {
	std::vector<std::string> path;
	Finder->getPath(path);
	for(size_t i = 0; i < path.size(); ++i) {
		scan(path[i]);
	}
	
	if (_maps.empty())
		throw_ex(("no maps found. sorry. install some maps/reinstall game."));
		
	std::sort(_maps.begin(), _maps.end());
		
	_upper_box = new UpperBox(w, 80, true);
	int xdummy, ybase;
	_upper_box->get_size(xdummy, ybase);
	ybase += 4;
	
	notepad = new Notepad(w, "medium");
	
	notepad->add("menu/modes", "deathmatch");
	notepad->add("menu/modes", "team-deathmatch");
	notepad->add("menu/modes", "cooperative");
	notepad->add("menu/modes", "capture-the-flag");
	
	GET_CONFIG_VALUE("menu.default-game-mode", int, dgm, 0);
	notepad->set(dgm);

	add(16, ybase, notepad);
	{
		int w, h;
		notepad->get_size(w, h);
		ybase += h;
	}
	ybase1 = ybase;

	sdlx::Rect list_pos(0, ybase, (w - 64)/3, h - 256);
	_list = new ScrollList(std::string(), "medium", list_pos.w, list_pos.h);
	add(list_pos.x, list_pos.y, _list);
	
	reload();

	sdlx::Rect map_pos(list_pos.w + 16, ybase, (w - 64) / 3, h - 256);

	_picker = NULL;
	TRY {
		_picker = new PlayerPicker(w - map_pos.x - map_pos.w - 16, h - 256);
		_picker->set(getCurrentMap());
		add(map_pos.x + map_pos.w + 16, ybase, _picker);
	} CATCH("PlayerPicker::ctor", {delete _picker; throw; });

	
	TRY {
		int cw, ch;
		_upper_box->get_size(cw, ch);
		add((w - cw) / 2, 0, _upper_box);
	} CATCH("StartServerMenu", {delete _upper_box; throw; });


	int ydummy;
	_list->get_size(xdummy, ydummy);
	ybase2 = ybase + ydummy;
	ybase += ydummy + 4;
	add(0, ybase, _mode_panel = new ModePanel(w));

	_details = NULL;	
	TRY {
		_details = new MapDetails(map_pos.w, map_pos.h);
		_details->set(getCurrentMap());
		add(map_pos.x, map_pos.y, _details);
	} CATCH("MapPicker::ctor", {delete _details; _details = NULL; throw; });
}

void MapPicker::fillSlots() const {
	if (PlayerManager->get_slots_count() < 1)
		return;

	bool split;
	Config->get("multiplayer.split-screen-mode", split, false);

	std::vector<SlotConfig> config;
	const MapDesc & map = getCurrentMap();
	MenuConfig->fill(map.name, _picker->getVariant(), config);
	int idx1 = -1, idx2 = -1;
	for(size_t i = 0; i < config.size(); ++i) {
		if (i >= (size_t)map.slots)
			break;
		
		PlayerSlot &slot = PlayerManager->get_slot(i);
		std::string type = config[i].type;
		if (type.empty() || type == "?")
			continue;
		std::string object = config[i].vehicle;
		if (object == "?") {
			static const char *objects[] = {"launcher", "shilka", "tank"};
			object = objects[mrt::random(sizeof(objects) / sizeof(objects[0]))];
		}
		
		std::string animation;
		mrt::to_lower(object);
		LOG_DEBUG(("before: %s:%s", object.c_str(), animation.c_str()));
		slot.getDefaultVehicle(object, animation);
		LOG_DEBUG(("after: %s:%s", object.c_str(), animation.c_str()));
		
		mrt::to_lower(type);
		std::string profile;
		Config->get("engine.profile", profile, std::string());
		if (profile.empty())
			throw_ex(("empty profile"));

		std::string cm = "ai";
		if (!split) {
			if (type == "player") {
				idx1 = i;
				Config->get("profile." + profile + ".control-method", cm, "keys");
				Config->get("profile." + profile + ".name", slot.name, Nickname::generate());
			} else 
				slot.name = Nickname::generate();
		} else {
			if (type == "player-1") {
				idx1 = i;
				Config->get("profile." + profile + ".control-method-1", cm, "keys-1");		
				Config->get("profile." + profile + ".name", slot.name, Nickname::generate());
			} else if (type == "player-2") {
				idx2 = i;
				Config->get("profile." + profile + ".control-method-2", cm, "keys-2");
				Config->get("profile." + profile + ".name-2", slot.name, Nickname::generate());
			} else 
				slot.name = Nickname::generate();
		}
		slot.createControlMethod(cm);
		slot.spawn_player(i, object, animation);
	}

	if (!split) {	
		PlayerManager->get_slot((idx1 == -1)?0:idx1).setViewport(Window->get_size());
	} else {
		v2<int> ts = Map->getTileSize();
		sdlx::Rect window_size = Window->get_size();
		int w = window_size.w / 2;

		sdlx::Rect vp1(window_size);
		sdlx::Rect vp2(window_size);
		vp1.w = w;

		vp2.x = w;
		vp2.w = w;
		LOG_DEBUG(("p1: %d %d %d %d", vp1.x, vp1.y, vp1.w, vp1.h));
		LOG_DEBUG(("p2: %d %d %d %d", vp2.x, vp2.y, vp2.w, vp2.h));
		PlayerManager->get_slot((idx1 == -1)?0:idx1).setViewport(vp1);
		PlayerManager->get_slot((idx2 == -1)?(idx1 != 1?1:0):idx2).setViewport(vp2); //avoid duplication of viewports
	}
	PlayerManager->validate_viewports();
}
