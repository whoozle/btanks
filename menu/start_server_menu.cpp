#include "start_server_menu.h"
#include "sdlx/surface.h"
#include "mrt/logger.h"
#include "mrt/exception.h"
#include "map_picker.h"
#include "upper_box.h"

StartServerMenu::StartServerMenu(const int w, const int h) : _w(w), _h(h) {
	TRY {
		_upper_box = new UpperBox; 
		_upper_box->init(500, 80, true);

		sdlx::Rect r((w - _upper_box->w) / 2, 32, _upper_box->w, _upper_box->h);
		add(r, _upper_box);
	} CATCH("StartServerMenu", {delete _upper_box; throw; });
	add(sdlx::Rect(0, 128, w, h - 128), _map_picker = new MapPicker(w, h - 128));
}

void StartServerMenu::tick(const float dt) {
	const MapPicker::MapDesc &map = _map_picker->getCurrentMap();
	_upper_box->value = map.game_type;
	Container::tick(dt);
}
