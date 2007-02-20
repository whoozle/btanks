#include "scroll_list.h"
#include "config.h"
#include "sdlx/rect.h"
#include "sdlx/surface.h"
#include "resource_manager.h"
#include <assert.h>
#include "math/unary.h"
#include "math/binary.h"

ScrollList::ScrollList(const int w, const int h) : _item_h(1), _client_w(64), _client_h(64), _pos(0), _vel(0), _current_item(0) {
	_background.init("menu/background_box.png", "menu/highlight_medium.png", w, h);
	GET_CONFIG_VALUE("engine.data-directory", std::string, data_dir, "data");
	_font.load(data_dir + "/font/medium.png", sdlx::Font::AZ09, true);
	_scrollers = ResourceManager->loadSurface("menu/v_scroller.png");

	_item_h = _font.getHeight() + 3;
}

void ScrollList::tick(const float dt) {
	int scroll_marg = _client_h / 3;
	int yp = _current_item * _item_h;
	if (_vel != 0) {
		if (math::abs((int)(math::max(yp - _client_h / 2, 0) - _pos)) < _item_h)
			_vel = 0;
	}
	if (yp < _pos + scroll_marg || yp > _pos + _client_h - scroll_marg) {
		_vel = 120 * math::sign<int>((int)(math::max(yp - _client_h / 2, 0) - _pos));
		_pos += _vel * dt;
	}
	if (_pos < 0) 
		_pos = 0;
	if (_pos + _client_h /2 > _list.size() * _item_h) 
		_pos = _list.size() * _item_h - _client_h;
	//LOG_DEBUG(("yp: %d, _pos : %g, (margin: %d)", yp, _pos, scroll_marg));
}


void ScrollList::render(sdlx::Surface &surface, const int x, const int y) {
	_background.render(surface, x, y);
	
	sdlx::Rect old_clip;
	surface.getClipRect(old_clip);
	
	
	int mx, my;
	_background.getMargins(mx, my);
	
	_client_h = _background.h - my * 2;
	_client_w = _background.w - mx * 2;

// scrollers' area

	int scroller_h = _scrollers->getHeight();
	int scroller_w = _scrollers->getWidth() / 6;
	
	_up_area = sdlx::Rect(_client_w + my - scroller_w, my, scroller_w, scroller_h);
	surface.copyFrom(*_scrollers, sdlx::Rect(0, 0, scroller_w, scroller_h), x + (int)_up_area.x, y + (int)_up_area.y);
	_down_area = sdlx::Rect(_up_area.x, my + _client_h - scroller_h, scroller_w, scroller_h);
	surface.copyFrom(*_scrollers, sdlx::Rect(scroller_w, 0, scroller_w, scroller_h), x + (int)_down_area.x, y + (int)_down_area.y);

//main list
	
	surface.setClipRect(sdlx::Rect(x + mx, y + my, _client_w, _client_h));

	assert(_client_h > 0);
	int p = ((int)_pos) / _item_h;
	int n = p + (_client_h - 1) / _item_h + 2;
	if (n > (int)_list.size()) 
		n = _list.size();
	
	int yp = my + y - ((int)_pos) % _item_h;
	for(; p < n; ++p) {
		if (p == _current_item) 
			_background.renderHL(surface, x - 3 * mx, yp + _item_h / 2);
		_font.render(surface, x + mx, yp, _list[p]);
		yp += _item_h;
	}

	surface.setClipRect(old_clip);
}

bool ScrollList::onKey(const SDL_keysym sym) {
	switch(sym.sym) {
	case SDLK_UP:
		if (_current_item > 0 ) 
			--_current_item;
		//LOG_DEBUG(("up: %u", _current_item));
		return true;

	case SDLK_DOWN:
		++_current_item;
		if (_current_item >= (int)_list.size()) 
			_current_item = _list.size() - 1;
		//LOG_DEBUG(("down: %u", _current_item));
		return true;
	default: 
		return false;
	}

	return false;
}

bool ScrollList::onMouse(const int button, const bool pressed, const int x, const int y) {
	//implement dragging of scroller here.
	
	if (!pressed)
		return false;
	
	if (_up_area.in(x, y)) {
		if (_current_item > 0 ) 
			--_current_item;
		//LOG_DEBUG(("up: %u", _current_item));
		return true;
	} else if (_down_area.in(x, y)) {
		++_current_item;
		if (_current_item >= (int)_list.size()) 
			_current_item = _list.size() - 1;
		//LOG_DEBUG(("down: %u", _current_item));
		return true;
	}
	return false;
}
