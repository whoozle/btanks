#include "start_server_menu.h"
#include "button.h"
#include "mrt/logger.h"
#include "menu.h"
#include "menu_config.h"
#include "map_picker.h"
#include "game.h"
#include "map_desc.h"
#include "player_manager.h"
#include "i18n.h"

StartServerMenu::StartServerMenu(MainMenu *parent, const int w, const int h) : _parent(parent)  {
	add(0, 0, _map_picker = new MapPicker(w, h));
	_back = new Button("big", I18n->get("menu", "back"));
	add(64, h - 96, _back);
	
	_start = new Button("big", I18n->get("menu", "start"));
	int bw, bh;
	_start->getSize(bw, bh);
	add(w - 64 - bw, h - 96, _start);
}

void StartServerMenu::start() {
	const MapDesc &map = _map_picker->getCurrentMap();
	if (map.slots < 1) {
		Game->displayMessage(I18n->get("menu", "no-slots-in-map"), 1);
		return;
	}

	LOG_DEBUG(("start multiplayer server requested"));
	Game->clear();
	Game->loadMap(map.name);
		
	_map_picker->fillSlots();
	
	PlayerManager->startServer();
	MenuConfig->save();

	_parent->back();
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
