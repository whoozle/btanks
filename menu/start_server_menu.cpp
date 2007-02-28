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

StartServerMenu::StartServerMenu(MainMenu *parent, const int w, const int h) : _parent(parent), _w(w), _h(h) {
	TRY {
		_upper_box = new UpperBox(500, 80, true);
		sdlx::Rect r((w - _upper_box->w) / 2, 32, _upper_box->w, _upper_box->h);
		add(r, _upper_box);
	} CATCH("StartServerMenu", {delete _upper_box; throw; });
	add(sdlx::Rect(0, 128, w, h - 128), _map_picker = new MapPicker(w, h - 128));
	_back = new Button("big", "BACK");
	int bw, bh;
	_back->getSize(bw, bh);
	add(sdlx::Rect(64, h - 96, bw, bh), _back);
	
	_start = new Button("big", "START");
	_start->getSize(bw, bh);
	add(sdlx::Rect(w - 64 - bw, h - 96, bw, bh), _start);
}

void StartServerMenu::tick(const float dt) {
	const MapPicker::MapDesc &map = _map_picker->getCurrentMap();
	_upper_box->value = map.game_type;
	Container::tick(dt);
	if (_back->clicked()) {
		LOG_DEBUG(("[back] clicked"));
		_back->reset();
		_parent->back();
	}
	if (_start->clicked()) {
		LOG_DEBUG(("[start] clicked..."));
		_start->reset();
		_parent->back();
		const MapPicker::MapDesc &map = _map_picker->getCurrentMap();

		LOG_DEBUG(("start multiplayer server requested"));
		Game->clear();
		Game->loadMap(map.name);

		GET_CONFIG_VALUE("player.control-method", std::string, cm, "keys");

		std::string vehicle, animation;
		PlayerManager->getDefaultVehicle(vehicle, animation);
		int idx = PlayerManager->spawnPlayer(vehicle, animation, cm);

		assert(idx == 0);

		PlayerManager->setViewport(idx, Game->getSize());
		PlayerManager->startServer();
	}
}
