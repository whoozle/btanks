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

void Hud::renderSplash(sdlx::Surface &window) const {
	int spx = (window.getWidth() - _splash.getWidth()) / 2;
	int spy = (window.getHeight() - _splash.getHeight()) / 2;
	
	window.copyFrom(_splash, spx, spy);
}


void Hud::renderLoadingBar(sdlx::Surface &window, const float progress) const {
	renderSplash(window);

	GET_CONFIG_VALUE("hud.loading-bar.position", float, yf, 2.0/3);
	GET_CONFIG_VALUE("hud.loading-bar.border-size", int, border, 3);
	
	int y = (int)(window.getHeight() * yf);
	int x = (window.getWidth() - _loading_border.getWidth()) / 2;
	
	window.copyFrom(_loading_border, x, y);
	assert(progress >= 0 && progress <= 1.0);
	int w = (int) (progress * (_loading_border.getWidth() - 2 * border));

	int i, n = w / _loading_item.getWidth();
	for(i = 0; i < n; ++i) {
		window.copyFrom(_loading_item, border + x + i * _loading_item.getWidth(), y + border);
	}
	w -= n * _loading_item.getWidth();
	sdlx::Rect src(0, 0, w, _loading_item.getHeight());
	window.copyFrom(_loading_item, src, border + x + i * _loading_item.getWidth(), y + border);
}


Hud::Hud(const int w, const int h) {
	GET_CONFIG_VALUE("engine.data-directory", std::string, data_dir, "data");
	_background.loadImage(data_dir + "/tiles/hud_line.png");
	_loading_border.loadImage(data_dir + "/tiles/loading_border.png");
	_loading_item.loadImage(data_dir + "/tiles/loading_item.png");
	_font.load(data_dir + "/font/big.png", sdlx::Font::AZ09);
	
	LOG_DEBUG(("searching splash... %dx%d", w, h));
	int sw = 0;
	int splash_sizes[] = { 1280, 1152, 1024, 800 };
	for(unsigned i = 0; i < sizeof(splash_sizes) / sizeof(splash_sizes[0]); ++i) {
		sw = w;
		if (w >= splash_sizes[i]) {
			break;
		}
	}
	LOG_DEBUG(("using splash %d", sw));
	_splash.loadImage(mrt::formatString("%s/tiles/splash_%d.png", data_dir.c_str(), sw));
}

Hud::~Hud() {}

