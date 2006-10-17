#include "hud.h"
#include "config.h"
#include "sdlx/font.h"
#include "player_manager.h"
#include "player_slot.h"
#include "object.h"

void Hud::render(sdlx::Surface &window) {
	window.copyFrom(_background, 0, 0);
	
	size_t n = PlayerManager->getSlotsCount();

	//only one visible player supported
	for(size_t i = 0; i < n; ++i) {
		const PlayerSlot &slot = PlayerManager->getSlot(i);
		if (!slot.visible)
			continue;
	
		std::string hp = mrt::formatString("HP%2d", slot.obj->hp);
		_font.render(window, slot.viewport.x, slot.viewport.y, hp);	
	}
}

Hud::Hud() {
	GET_CONFIG_VALUE("engine.data-directory", std::string, data_dir, "data");
	_background.loadImage(data_dir + "/tiles/hud_line.png");
	_font.load(data_dir + "/font/big.png", sdlx::Font::AZ09);
}

Hud::~Hud() {}

