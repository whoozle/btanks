#include "scroll_list.h"
#include "config.h"
#include "sdlx/rect.h"
#include "sdlx/surface.h"
#include "resource_manager.h"
#include <assert.h>
#include "math/unary.h"
#include "math/binary.h"

ScrollList::ScrollList(const int w, const int h) : _item_h(0), _client_w(64), _client_h(64), _pos(0), _vel(0), _current_item(0) {
	_background.init("menu/background_box.png", "menu/highlight_medium.png", w, h);
	_font = ResourceManager->loadFont("medium", true);
	_scrollers = ResourceManager->loadSurface("menu/v_scroller.png");

	_item_h = _font->getHeight() + 5;
}

void ScrollList::add(const std::string &item) {
	_list.push_back(item);
}

void ScrollList::clear() {
	_list.clear();
}


void ScrollList::tick(const float dt) {
	if (_list.empty() || _item_h == 0)
		return;
	
	int scroll_marg = _client_h / 3;
	int yp = _current_item * _item_h;
	if (_vel != 0) {
		if (math::abs((int)(math::max<int>(yp - _client_h / 2, 0) - _pos)) < _item_h)
			_vel = 0;
	}

	if (yp < _pos + scroll_marg || yp > _pos + _client_h - scroll_marg) {
		int dpos = (int)(math::max<int>(yp - _client_h / 2, 0) - _pos);
		_vel = 120 * math::sign<int>(dpos);
		_pos += _vel * dt;
	}

	if (_pos  > _list.size() * _item_h - _client_h) {
		_pos = _list.size() * _item_h - _client_h;
		_vel = 0;
	}

	if (_pos < 0) {
		_pos = 0;
		_vel = 0;
	}

}


void ScrollList::render(sdlx::Surface &surface, const int x, const int y) {
	_background.render(surface, x, y);
	
	sdlx::Rect old_clip;
	surface.getClipRect(old_clip);
	
	
	int mx, my;
	_background.getMargins(mx, my);
	
	_client_w = _background.w - mx * 2;
	_client_h = _background.h - my * 2;

// scrollers' area

	int scroller_h = _scrollers->getHeight();
	int scroller_w = _scrollers->getWidth() / 6;
	
	_up_area = sdlx::Rect(_client_w + my - scroller_w, my, scroller_w, scroller_h);
	surface.copyFrom(*_scrollers, sdlx::Rect(0, 0, scroller_w, scroller_h), x + (int)_up_area.x, y + (int)_up_area.y);
	_down_area = sdlx::Rect(_up_area.x, my + _client_h - scroller_h, scroller_w, scroller_h);
	surface.copyFrom(*_scrollers, sdlx::Rect(scroller_w, 0, scroller_w, scroller_h), x + (int)_down_area.x, y + (int)_down_area.y);
	_items_area = sdlx::Rect(mx, my, _client_w - 2 * mx, _client_h);

	if (_list.empty())
		return;
//main list
	
	surface.setClipRect(sdlx::Rect(x + mx, y + my, _items_area.w, _items_area.h));

	assert(_client_h > 0);
	int p = ((int)_pos) / _item_h;
	int n = p + (_client_h - 1) / _item_h + 2;
	if (n > (int)_list.size()) 
		n = _list.size();
	assert(p>= 0 && p < (int)_list.size());
	
	int yp = my + y - ((int)_pos) % _item_h;
	for(; p < n; ++p) {
		if (p == (int)_current_item) 
			_background.renderHL(surface, x - 3 * mx, yp + _item_h / 2);
		_font->render(surface, x + mx, yp, _list[p]);
		yp += _item_h;
	}

	surface.setClipRect(old_clip);
}

bool ScrollList::onKey(const SDL_keysym sym) {
	switch(sym.sym) {
	case SDLK_PAGEUP:
		_current_item -= 9;
	case SDLK_UP:
		--_current_item;
		if (_current_item < 0 ) 
			_current_item = 0;
		//LOG_DEBUG(("up: %u", _current_item));
		return true;

	case SDLK_PAGEDOWN:
		_current_item += 9;
	case SDLK_DOWN:
		++_current_item;
		if (_current_item >= (int)_list.size()) 
			_current_item = (int)_list.size() - 1;
		//LOG_DEBUG(("down: %u", _current_item));
		return true;
	default: 
		return false;
	}

	return false;
}

bool ScrollList::onMouse(const int button, const bool pressed, const int x, const int y) {
	//implement dragging of scroller here.
	//LOG_DEBUG(("boo %d %d %d %d", button, pressed, x, y));
	
	if (!pressed || button == SDL_BUTTON_MIDDLE) //skip accidental wheel clicks
		return false;

	int mx, my;
	_background.getMargins(mx, my);
	
	if (_items_area.in(x, y)) {
		if (button == SDL_BUTTON_WHEELUP)
			goto up;
		if (button == SDL_BUTTON_WHEELDOWN)
			goto down;
		//LOG_DEBUG(("%d %d -> %d", x, y, y + (int)_pos - my));
		int item = (y - my + (int)_pos) / _item_h;
		if (item >= 0 && item < (int)_list.size())
			_current_item = item;
		return true;
	}	
	
	if (_up_area.in(x, y)) {
	up: //fix it 
		if (_current_item > 0 ) 
			--_current_item;
		//LOG_DEBUG(("up: %u", _current_item));
		return true;
	} else if (_down_area.in(x, y)) {
	down: 
		++_current_item;
		if (_current_item >= (int)_list.size()) 
			_current_item = (int)_list.size() - 1;
		//LOG_DEBUG(("down: %u", _current_item));
		return true;
	} else 
	return false;
}
