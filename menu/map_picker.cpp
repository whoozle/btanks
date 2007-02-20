#include "map_picker.h"
#include "scroll_list.h"
#include "mrt/exception.h"
#include "mrt/directory.h"
#include "config.h"

void MapPicker::loadScreenshot() {
	GET_CONFIG_VALUE("engine.data-directory", std::string, data_dir, "data");
	TRY {
		_screenshot.loadImage(_maps[_index].first + "/maps/" + _maps[_index].second + ".jpg");
		_screenshot.convertAlpha();
	} CATCH("loading screenshot", {});
}

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

	sdlx::Rect list_pos(0, 0, (w - 64)/3, h / 2);
	list_pos.x = 64;
	list_pos.y = 64;
	ScrollList *list = NULL;
	TRY {
		list = new ScrollList(list_pos.w, list_pos.h);
		for(MapList::const_iterator i = _maps.begin(); i != _maps.end(); ++i) {
			list->add(i->second);
		}
		add(list_pos, list);
	} CATCH("MapPicker::ctor", {delete list; throw; });
	
	
}
