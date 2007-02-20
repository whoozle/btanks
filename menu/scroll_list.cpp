#include "scroll_list.h"
#include "config.h"
#include "sdlx/rect.h"
#include "sdlx/surface.h"

ScrollList::ScrollList(const int w, const int h) : _pos(0) {
	_background.init("menu/background_box.png", w, h);
	GET_CONFIG_VALUE("engine.data-directory", std::string, data_dir, "data");
	_font.load(data_dir + "/font/medium.png", sdlx::Font::AZ09, true);
}

void ScrollList::render(sdlx::Surface &surface, const int x, const int y) {
	_background.render(surface, x, y);
	
	sdlx::Rect old_clip;
	surface.getClipRect(old_clip);
	
	int item_h = _font.getHeight() + 3;
	
	int mx, my;
	_background.getMargins(mx, my);
	
	int client_h = _background.h - my * 2, client_w = _background.w - mx * 2;
	surface.setClipRect(sdlx::Rect(x + mx, y + my, client_w, client_h));
	
	int n = (client_h - 1)/ item_h + 2;
	if (n >= (int)_list.size()) 
		n = _list.size() - 1;
	
	int yp = my + y;
	for(int p = _pos / item_h; p < n; ++p) {
		_font.render(surface, x + mx, yp, _list[p]);
		yp += item_h;
	}

	surface.setClipRect(old_clip);
}

bool ScrollList::onKey(const SDL_keysym sym) {
	return false;
}

bool ScrollList::onMouse(const int button, const bool pressed, const int x, const int y) {
	LOG_DEBUG(("aaa!"));
	return false;
}
