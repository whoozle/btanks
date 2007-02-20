#include "scroll_list.h"
#include "config.h"
#include "sdlx/rect.h"
#include "sdlx/surface.h"
#include "resource_manager.h"

ScrollList::ScrollList(const int w, const int h) : _pos(0), _current_item(0) {
	_background.init("menu/background_box.png", w, h);
	GET_CONFIG_VALUE("engine.data-directory", std::string, data_dir, "data");
	_font.load(data_dir + "/font/medium.png", sdlx::Font::AZ09, true);
	_scrollers = ResourceManager->loadSurface("menu/v_scroller.png");
}

void ScrollList::render(sdlx::Surface &surface, const int x, const int y) {
	_background.render(surface, x, y);
	
	sdlx::Rect old_clip;
	surface.getClipRect(old_clip);
	
	int item_h = _font.getHeight() + 3;
	
	int mx, my;
	_background.getMargins(mx, my);
	
	int client_h = _background.h - my * 2, client_w = _background.w - mx * 2;

// scrollers' area

	int scroller_h = _scrollers->getHeight();
	int scroller_w = _scrollers->getWidth() / 6;
	
	_up_area = sdlx::Rect(client_w + my - scroller_w, my, scroller_w, scroller_h);
	surface.copyFrom(*_scrollers, sdlx::Rect(0, 0, scroller_w, scroller_h), x + (int)_up_area.x, y + (int)_up_area.y);
	_down_area = sdlx::Rect(_up_area.x, my + client_h - scroller_h, scroller_w, scroller_h);
	surface.copyFrom(*_scrollers, sdlx::Rect(scroller_w, 0, scroller_w, scroller_h), x + (int)_down_area.x, y + (int)_down_area.y);

//main list
	
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
	//implement dragging of scroller here.
	
	if (!pressed)
		return false;
	
	if (_up_area.in(x, y)) {
		if (_current_item > 0 ) 
			--_current_item;
		return true;
	} else if (_down_area.in(x, y)) {
		++_current_item;
		if (_current_item >= _list.size()) 
			_current_item = _list.size() - 1;
		return true;
	}
	return false;
}
