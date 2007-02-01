#include "mapitem.h"
#include "config.h"
#include <algorithm>
#include "mrt/directory.h"

MapItem::MapItem(sdlx::TTF &font, const std::string &name) :
 MenuItem(font, name, "text", std::string(), std::string()), _active(false) {
	//load map.
	GET_CONFIG_VALUE("engine.data-directory", std::string, data_dir, "data");
	
	mrt::Directory dir;
	const std::string path = data_dir + "/maps";
	dir.open(path);
	std::string map;
	while(!(map = dir.read()).empty()) {
		mrt::toLower(map);
		if (map.size() < 5 || map.substr(map.size() - 4) != ".tmx")
			continue;
		map = map.substr(0, map.size() - 4);
		LOG_DEBUG(("found map: %s", map.c_str()));
		_maps.push_back(map);
	}	
	dir.close();
	
	if (_maps.empty())
		throw_ex(("no maps found. sorry. install some maps/reinstall game."));
		
	std::sort(_maps.begin(), _maps.end());

	Config->get("menu.default-mp-map", map, "survival");
	for(_index = 0; _index < _maps.size(); ++_index) {
		if (_maps[_index] == map)
			break;
	}
	if (_index >= _maps.size())
		_index = 0;
	LOG_DEBUG(("map index: %d", _index));
	updateValue();
}

void MapItem::render(sdlx::Surface &dst, const int x, const int y) {
	if (_active) {
		if (!_screenshot.isNull()) {
			int w, h;
			getSize(w, h);
			dst.copyFrom(_screenshot, x + w / 2 - _screenshot.getWidth() / 2 , y - _screenshot.getHeight() - 10);
		}
	}
	
	MenuItem::render(dst, x, y);
}

void MapItem::loadScreenshot() {
	GET_CONFIG_VALUE("engine.data-directory", std::string, data_dir, "data");
	TRY {
		_screenshot.loadImage(data_dir + "/maps/" + _maps[_index] + ".jpg");
		_screenshot.convertAlpha();
	} CATCH("loading screenshot", {});
}

void MapItem::onClick() {
	//LOG_DEBUG(("starting map picker"));
	_active = true;
	loadScreenshot();
}

void MapItem::finish() {
	//LOG_DEBUG(("exiting map picker"));
	_active = false;
	_screenshot.free();
}

void MapItem::updateValue() {
	_text = _value = _maps[_index];
	mrt::toUpper(_text);
	for(size_t i = 0; i < _text.size(); ++i) {
		if (_text[i] == '_') 
			_text[i] = ' ';
	}
	_text = "MAP: " + _text;
	Config->set("menu.default-mp-map", _value);
	MenuItem::render();
	loadScreenshot();
}


const bool MapItem::onKey(const SDL_keysym sym) {
	if (!_active)
		return false;
	
	switch(sym.sym) {
	case SDLK_RIGHT: 
		_index = (_index + 1) % _maps.size();
		updateValue();
		break;
	case SDLK_LEFT: 
		_index = (_index + _maps.size() - 1) % _maps.size();
		updateValue();
		break;
	case SDLK_ESCAPE: 
		finish();
		break;
	case SDLK_RETURN: 
		finish();
		break;
	default: ;	
	}
	return true;
}
