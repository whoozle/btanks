#include "map_picker.h"
#include "map_details.h"
#include "scroll_list.h"
#include "mrt/exception.h"
#include "mrt/directory.h"
#include "config.h"

void MapPicker::scan(const std::string &path) {
	if (!mrt::Directory::exists(path))
		return;
	
	mrt::Directory dir;
	dir.open(path);

	std::string map;
	while(!(map = dir.read()).empty()) {
		mrt::toLower(map);
		if (map.size() < 5 || map.substr(map.size() - 4) != ".tmx")
			continue;
		map = map.substr(0, map.size() - 4);
		LOG_DEBUG(("found map: %s", map.c_str()));
		_maps.push_back(MapList::value_type(path, map));
	}	
	dir.close();

}

void MapPicker::tick(const float dt) {
	if (_index != _list->getPosition()) {
		_index = _list->getPosition();
		_details->set(_maps[_index].first, _maps[_index].second, "blah blah blah");
	}
	Container::tick(dt);
}


MapPicker::MapPicker(const int w, const int h) : _index(0) {
	GET_CONFIG_VALUE("engine.data-directory", std::string, data_dir, "data");

	scan(data_dir + "/maps");
	scan("private/" + data_dir + "/maps");
	
	if (_maps.empty())
		throw_ex(("no maps found. sorry. install some maps/reinstall game."));
		
	std::sort(_maps.begin(), _maps.end());
	
	std::string map;

	Config->get("menu.default-mp-map", map, "lenin_square");
	for(_index = 0; _index < _maps.size(); ++_index) {
		if (_maps[_index].second == map)
			break;
	}
	if (_index >= _maps.size())
		_index = 0;
	LOG_DEBUG(("map index: %d", _index));

	sdlx::Rect list_pos(0, 0, (w - 64)/3, h - 128);
	_list = NULL;
	TRY {
		_list = new ScrollList(list_pos.w, list_pos.h);
		for(MapList::const_iterator i = _maps.begin(); i != _maps.end(); ++i) {
			_list->add(i->second);
		}
		add(list_pos, _list);
		_list->setPosition(_index);
	} CATCH("MapPicker::ctor", {delete _list; throw; });

	sdlx::Rect map_pos(list_pos.w + 16, 0, (w - 64) / 3, h - 128);

	_details = NULL;	
	TRY {
		_details = new MapDetails(map_pos.w, map_pos.h);
		_details->set(_maps[_index].first, _maps[_index].second, "blah blah");
		add(map_pos, _details);
	} CATCH("MapPicker::ctor", {delete _details; throw; });

}
