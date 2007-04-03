#include <assert.h>
#include <algorithm>

#include "map_picker.h"
#include "map_details.h"
#include "player_picker.h"
#include "scroll_list.h"
#include "i18n.h"
#include "player_manager.h"
#include "map_desc.h"
#include "upper_box.h"
#include "game.h"
#include "menu_config.h"
#include "finder.h"

#include "mrt/exception.h"
#include "mrt/directory.h"
#include "mrt/xml.h"
#include "mrt/random.h"

#include "config.h"

#include "tmx/map.h"

struct MapScanner : mrt::XMLParser {
	MapScanner() : slots(0) {}
	int slots;
	std::string object_restriction;
	std::string game_type;

	void scan(const std::string &name) {
		parseFile(name);
		LOG_DEBUG(("parser: slots: %d, object_restriction: '%s'", slots, object_restriction.c_str()));
	}
private: 
	virtual void start(const std::string &name, Attrs &attr) {
		if (name == "property") {
			if (attr["name"].substr(0, 6) == "spawn:")
				++slots;
			else if (attr["name"] == "config:multiplayer.restrict-start-vehicle" && attr["value"].substr(0, 7) == "string:") {
				object_restriction = attr["value"].substr(7);
			} else if (attr["name"] == "config:multiplayer.game-type" && attr["value"].substr(0, 7) == "string:") {
				game_type = attr["value"].substr(7);
			}
		}
	}
	virtual void end(const std::string &name) {}
//	virtual void charData(const std::string &data);
};


void MapPicker::scan(const std::string &path) {
	if (!mrt::Directory::exists(path))
		return;
	
	mrt::Directory dir;
	dir.open(path);
	std::string fname;

	while(!(fname = dir.read()).empty()) {
		std::string map = fname;
		
		mrt::toLower(map);
		if (map.size() < 5 || map.substr(map.size() - 4) != ".tmx")
			continue;
		map = map.substr(0, map.size() - 4);
		LOG_DEBUG(("found map: %s", map.c_str()));
		MapScanner m;
		TRY {
			m.scan(path + "/" + fname);
		} CATCH("scanning map", {});
		const std::string &comments = I18n->has("maps/descriptions", map)?I18n->get("maps/descriptions", map): 
			I18n->get("maps/descriptions", "(default)");
		_maps.push_back(MapList::value_type(path, map, comments, m.object_restriction, m.game_type, m.slots));
	}	
	dir.close();

}

void MapPicker::tick(const float dt) {
	const MapDesc &map = getCurrentMap();
	_upper_box->value = map.game_type;

	if (_upper_box->changed() || _index != _list->get()) {
		_upper_box->reset();

		_index = _list->get();

		Config->set("menu.default-mp-map", _maps[_index].name);
		_details->set(_maps[_index].base, _maps[_index].name, _maps[_index].desc );
		_picker->set(_maps[_index]);
	}
	Container::tick(dt);
}


MapPicker::MapPicker(const int w, const int h) : _index(0) {
	std::vector<std::string> path;
	Finder->getPath(path);
	for(size_t i = 0; i < path.size(); ++i) {
		scan(path[i] + "/maps");
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
	
	TRY {
		_upper_box = new UpperBox(500, 80, true);
		add((w - _upper_box->w) / 2, 32, _upper_box);
	} CATCH("StartServerMenu", {delete _upper_box; throw; });

	sdlx::Rect list_pos(0, 128, (w - 64)/3, h - 256);
	_list = NULL;
	TRY {
		_list = new ScrollList("medium", list_pos.w, list_pos.h);
		for(MapList::const_iterator i = _maps.begin(); i != _maps.end(); ++i) {
			_list->add(i->name);
		}
		add(list_pos.x, list_pos.y, _list);
		_list->set(_index);
	} CATCH("MapPicker::ctor", {delete _list; throw; });

	sdlx::Rect map_pos(list_pos.w + 16, 128, (w - 64) / 3, h - 256);

	_picker = NULL;
	TRY {
		_picker = new PlayerPicker(w - map_pos.x - map_pos.w - 16, h - 256);
		_picker->set(_maps[_index]);
		add(map_pos.x + map_pos.w + 16, 128, _picker);
	} CATCH("PlayerPicker::ctor", {delete _picker; throw; });

	_details = NULL;	
	TRY {
		_details = new MapDetails(map_pos.w, map_pos.h);
		_details->set(_maps[_index].base, _maps[_index].name, _maps[_index].desc);
		add(map_pos.x, map_pos.y, _details);
	} CATCH("MapPicker::ctor", {delete _details; throw; });


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
		PlayerManager->getDefaultVehicle(object, animation);
		LOG_DEBUG(("after: %s:%s", object.c_str(), animation.c_str()));
		
		mrt::toLower(type);
		if (type == "ai")
			object = "ai-" + object;

		std::string cm = "ai";
		if (!split) {
			if (type == "player") {
				idx1 = i;
				Config->get("player.control-method", cm, "keys");
			}
		} else {
			if (type == "player-1") {
				idx1 = i;
				Config->get("player.control-method-1", cm, "keys-1");		
			}
			if (type == "player-2") {
				idx2 = i;
				Config->get("player.control-method-2", cm, "keys-2");
			}
		}
		PlayerManager->createControlMethod(slot, cm);
		PlayerManager->spawnPlayer(slot, object, animation);
	}

	if (!split) {	
		PlayerManager->setViewport((idx1 == -1)?0:idx1, Game->getSize());
	} else {
		v2<int> ts = Map->getTileSize();
		int w = Game->getSize().w / 2;

		sdlx::Rect vp1(Game->getSize());
		sdlx::Rect vp2(Game->getSize());
		vp1.w = w;

		vp2.x = w;
		vp2.w = w;
		LOG_DEBUG(("p1: %d %d %d %d", vp1.x, vp1.y, vp1.w, vp1.h));
		LOG_DEBUG(("p2: %d %d %d %d", vp2.x, vp2.y, vp2.w, vp2.h));
		PlayerManager->setViewport((idx1 == -1)?0:idx1, vp1);
		PlayerManager->setViewport((idx2 == -1)?(idx1 != 1?1:0):idx2, vp2); //avoid duplication of viewports
	}
	PlayerManager->validateViewports();
}
