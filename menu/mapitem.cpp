#include "mapitem.h"
#include "config.h"
#include <algorithm>

MapItem::MapItem(sdlx::TTF &font, const std::string &name) :
 MenuItem(font, name, "text", std::string(), std::string()), _active(false) {
	//load map.
	std::string map;
	Config->get("menu.default-mp-map", map, "survival");
	_text = _value = map;
	std::transform(_text.begin(), _text.end(), _text.begin(), toupper);
	_text = "MAP: " + _text;
	render();
}


void MapItem::onClick() {
	_active = true;
	LOG_DEBUG(("starting map picker"));
}

void MapItem::finish() {
	LOG_DEBUG(("exiting map picker"));
	_active = false;
}

const bool MapItem::onKey(const Uint8 type, const SDL_keysym sym) {
	if (!_active || type != SDL_KEYDOWN)
		return false;
	
	switch(sym.sym) {
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
