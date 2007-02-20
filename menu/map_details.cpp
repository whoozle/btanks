#include "map_details.h"
#include "mrt/exception.h"
#include "config.h"

MapDetails::MapDetails(const int w, const int h) {
	_background.init("menu/background_box.png", w, h);
	GET_CONFIG_VALUE("engine.data-directory", std::string, data_dir, "data");
	_font.load(data_dir + "/font/small.png", sdlx::Font::AZ09, true);
}

void MapDetails::set(const std::string &base, const std::string &map, const std::string &comments) {
	TRY {
		_screenshot.free();
		_screenshot.loadImage(base + "/" + map + ".jpg");
		_screenshot.convertAlpha();
	} CATCH("loading screenshot", {});
	_comments = comments;
}

void MapDetails::render(sdlx::Surface &surface, const int x, const int y) {
	_background.render(surface, x, y);
	int mx, my;
	_background.getMargins(mx, my);
	
	int yp = my * 3 / 2;
	if (!_screenshot.isNull()) {
		int xs = (_background.w - _screenshot.getWidth()) / 2;
		surface.copyFrom(_screenshot, x + xs, y + yp);
		yp += _screenshot.getHeight() + 16;
	}
	_font.render(surface, x + mx, y + yp, _comments);
}

bool MapDetails::onKey(const SDL_keysym sym) {
	return false;
}
bool MapDetails::onMouse(const int button, const bool pressed, const int x, const int y) {
	return false;
}

