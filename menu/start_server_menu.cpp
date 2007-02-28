#include "start_server_menu.h"
#include "sdlx/surface.h"
#include "mrt/logger.h"
#include "mrt/exception.h"
#include "map_picker.h"
#include "upper_box.h"
#include "button.h"
#include "menu.h"
#include "game.h"
#include "config.h"
#include "player_manager.h"
#include "menu_config.h"
#include "map_desc.h"

StartServerMenu::StartServerMenu(MainMenu *parent, const int w, const int h) : _parent(parent), _w(w), _h(h) {
	add(sdlx::Rect(0, 0, w, h - 128), _map_picker = new MapPicker(w, h));
	_back = new Button("big", "BACK");
	int bw, bh;
	_back->getSize(bw, bh);
	add(sdlx::Rect(64, h - 96, bw, bh), _back);
	
	_start = new Button("big", "START");
	_start->getSize(bw, bh);
	add(sdlx::Rect(w - 64 - bw, h - 96, bw, bh), _start);
}

void StartServerMenu::tick(const float dt) {
	Container::tick(dt);
	if (_back->clicked()) {
		LOG_DEBUG(("[back] clicked"));
		_back->reset();
		_parent->back();
		MenuConfig->save();
	}
	if (_start->clicked()) {
		LOG_DEBUG(("[start] clicked..."));
		_start->reset();
		_parent->back();
		const MapDesc &map = _map_picker->getCurrentMap();

		LOG_DEBUG(("start multiplayer server requested"));
		Game->clear();
		Game->loadMap(map.name);
		
		_map_picker->fillSlots();
		
		PlayerManager->startServer();
		MenuConfig->save();
	}

}

StartServerMenu::~StartServerMenu() {}
