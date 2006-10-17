#include "hud.h"
#include "config.h"
#include "sdlx/font.h"
#include "player_manager.h"
#include "player_slot.h"
#include "object.h"

void Hud::render(sdlx::Surface &window) const {
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

void Hud::renderLoadingBar(sdlx::Surface &window, const float progress) const {
	GET_CONFIG_VALUE("hud.loading-bar.position", float, yf, 2.0/3);
	GET_CONFIG_VALUE("hud.loading-bar.border-size", int, border, 2);
	
	int y = (int)(window.getHeight() * yf);
	int x = (window.getWidth() - _loading_border.getWidth()) / 2;
	
	window.copyFrom(_loading_border, x, y);
	assert(progress >= 0 && progress <= 1.0);
	int n = (int) (progress * (_loading_border.getWidth() - 2 * border) / _loading_item.getWidth());
	for(int i = 0; i < n; ++i) {
		window.copyFrom(_loading_item, border + x + i * _loading_item.getWidth(), y + border);
	}
}


Hud::Hud() {
	GET_CONFIG_VALUE("engine.data-directory", std::string, data_dir, "data");
	_background.loadImage(data_dir + "/tiles/hud_line.png");
	_loading_border.loadImage(data_dir + "/tiles/loading_border.png");
	_loading_item.loadImage(data_dir + "/tiles/loading_item.png");
	_font.load(data_dir + "/font/big.png", sdlx::Font::AZ09);
}

Hud::~Hud() {}

