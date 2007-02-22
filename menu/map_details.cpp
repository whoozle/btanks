#include "map_details.h"
#include "mrt/exception.h"
#include "config.h"
#include "resource_manager.h"
#include "mrt/fs_node.h"

MapDetails::MapDetails(const int w, const int h) {
	_background.init("menu/background_box.png", w, h);

	GET_CONFIG_VALUE("engine.data-directory", std::string, data_dir, "data");
	_null_screenshot.loadImage(data_dir + "/maps/null.png");

	_font = ResourceManager->loadFont("small", true);
}

void MapDetails::set(const std::string &base, const std::string &map, const std::string &comments) {
	TRY {
		_screenshot.free();
		const std::string fname = base + "/" + map + ".jpg";
		if (mrt::FSNode::exists(fname)) {
			_screenshot.loadImage(fname);
			_screenshot.convertAlpha();
		}
	} CATCH("loading screenshot", {});
	_comments = comments;
}

void MapDetails::render(sdlx::Surface &surface, const int x, const int y) {
	_background.render(surface, x, y);
	int mx, my;
	_background.getMargins(mx, my);
	
	int yp = my * 3 / 2;

	const sdlx::Surface &screenshot = _screenshot.isNull()?_null_screenshot:_screenshot;
	int xs = (_background.w - screenshot.getWidth()) / 2;
	surface.copyFrom(screenshot, x + xs, y + yp);
	int ys = screenshot.getHeight();
	yp += (ys < 152)?152:ys;

	_font->render(surface, x + mx, y + yp, _comments);
}

bool MapDetails::onKey(const SDL_keysym sym) {
	return false;
}
bool MapDetails::onMouse(const int button, const bool pressed, const int x, const int y) {
	return false;
}

