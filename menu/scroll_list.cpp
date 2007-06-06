
/* Battle Tanks Game
 * Copyright (C) 2006-2007 Battle Tanks team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <string>
#include <stdexcept>
#include "scroll_list.h"
#include "config.h"
#include "sdlx/rect.h"
#include "sdlx/surface.h"
#include "resource_manager.h"
#include <assert.h>
#include "math/unary.h"
#include "math/binary.h"
#include "menu/label.h"

ScrollList::ScrollList(const std::string &background, const std::string &font, const int w, const int h, const int spacing) : 
_client_w(64), _client_h(64), _pos(0), _vel(0), _current_item(0), _spacing(spacing) {
	_background.init(background, "menu/highlight_medium.png", w, h);
	_font = ResourceManager->loadFont(font, true);
	_scrollers = ResourceManager->loadSurface("menu/v_scroller.png");
}

const std::string ScrollList::getValue() const { 
	Control *c = _list[_current_item]; 
	Label *l = dynamic_cast<Label *>(c);
	if (l == NULL)
		throw_ex(("cannot getValue from item %d", _current_item));
	return l->get();
}

void ScrollList::append(const std::string &item) {
	append(new Label(_font, item));
}

void ScrollList::append(Control *control) {
	_list.push_back(control);
	invalidate();
}

const int ScrollList::getItemY(const int idx) const {
	int y = 0;
	for(int i = 0; i < idx; ++i) {
		int w, h;
		_list[i]->getSize(w, h);
		h += _spacing;
		y += h;
	}
	return y;
}

const int ScrollList::getItemIndex(const int yp) const {
	int y = - _spacing/2;
	for(int i = 0; i < (int)_list.size(); ++i) {
		int w, h;
		_list[i]->getSize(w, h);
		h += _spacing;
		if (yp >= y && yp < y + h)
			return i; 
		y += h;
	}
	return _list.size() - 1;
}

void ScrollList::tick(const float dt) {
	Container::tick(dt);
	if (_list.empty())
		return;
	
	int scroll_marg = _client_h / 3;
	int yp = getItemY(_current_item);
	
	if (_vel != 0) {
		if (math::abs((int)(math::max<int>(yp - _client_h / 2, 0) - _pos)) < 8)
			_vel = 0;
	}

	if (yp < _pos + scroll_marg || yp > _pos + _client_h - scroll_marg) {
		int dpos = (int)(math::max<int>(yp - _client_h / 2, 0) - _pos);
		_vel = 200 * math::sign<int>(dpos);
		_pos += _vel * dt;
	}

	int h = getItemY(_list.size());
	
	if (_pos  >  h - _client_h) {
		_pos = h - _client_h;
		_vel = 0;
	}

	if (_pos < 0) {
		_pos = 0;
		_vel = 0;
	}

	for(List::iterator i = _list.begin(); i != _list.end(); ++i) {
		(*i)->tick(dt);
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
	//int p = 0;
	int p = getItemIndex((int)_pos);
	//int n = p + (_client_h - 1) / _item_h + 2;
	//if (n > (int)_list.size()) 
	int n = _list.size();
	assert(p>= 0 && p < (int)_list.size());
	
	int item_pos = getItemY(p);
	int yp = my + y - ((int)_pos - item_pos);
	for(; p < n; ++p) {
		int w, h;
		_list[p]->getSize(w, h);
		h += _spacing;

		if (p == (int)_current_item) {
			_background.renderHL(surface, x - 3 * mx, yp + h / 2 - _spacing / 2 + 1);
		}
		//_font->render(surface, x + mx, yp, _list[p]);
		_list[p]->render(surface, x + mx, yp);
		yp += h;
	}

	surface.setClipRect(old_clip);
	Container::render(surface, x, y);
}

bool ScrollList::onKey(const SDL_keysym sym) {
	if (Container::onKey(sym))
		return true;
	
	switch(sym.sym) {
	case SDLK_PAGEUP:
		if (_current_item > 0)
			invalidate();
		_current_item -= 9;
	case SDLK_UP:
		if (_current_item > 0)
			invalidate(true);
		--_current_item;
		if (_current_item < 0 ) 
			_current_item = 0;
		//LOG_DEBUG(("up: %u", _current_item));
		return true;

	case SDLK_HOME: 
		if (_current_item > 0)
			invalidate(true);
		_current_item = 0;
		return true;

	case SDLK_END: 
		if (_current_item != (int)_list.size() - 1)
			invalidate(true);
		_current_item = (int)_list.size() - 1;
		return true;

	case SDLK_PAGEDOWN:
		if (_current_item != (int)_list.size() - 1)
			invalidate();
		_current_item += 9;
	case SDLK_DOWN:
		if (_current_item != (int)_list.size() - 1)
			invalidate(true);
		++_current_item;
		if (_current_item >= (int)_list.size()) 
			_current_item = (int)_list.size() - 1;
		//LOG_DEBUG(("down: %u", _current_item));
		return true;
	default: 
		//LOG_DEBUG(("%d", sym.sym));
		size_t i;
		int c = tolower(sym.sym);
		for(i = 0; i < _list.size(); ++i) {
			Label *l = dynamic_cast<Label *>(_list[i]);
			if (l != NULL && !l->get().empty()) {
				int fc = tolower(l->get()[0]);
				if (fc == c) 
					break;
			}
		}
		if (i < _list.size()) {
			if (_current_item != (int)i)
				invalidate(true);
			_current_item = i;
			return true;
		}
		return false;
	}

	return false;
}

void ScrollList::up() {
		if (_current_item > 0 ) {
			--_current_item;
			invalidate(true);
		}
		//LOG_DEBUG(("up: %u", _current_item));
}

void ScrollList::down() {
		if (_current_item + 1 < (int)_list.size()) {
			++_current_item;
			invalidate(true);
		}
		//LOG_DEBUG(("down: %u", _current_item));
}

bool ScrollList::onMouse(const int button, const bool pressed, const int x, const int y) {
	if (Container::onMouse(button, pressed, x, y))
		return true;
	//implement dragging of scroller here.
	//LOG_DEBUG(("boo %d %d %d %d", button, pressed, x, y));
	
	if (button == SDL_BUTTON_MIDDLE) //skip accidental wheel clicks
		return false;

	if (button == SDL_BUTTON_WHEELUP) {
		if (pressed)
			up();
		return true;
	}

	if (button == SDL_BUTTON_WHEELDOWN) {
		if (pressed)
			down();
		return true;
	}


	int mx, my;
	_background.getMargins(mx, my);
	
	if (_items_area.in(x, y)) {
		//LOG_DEBUG(("%d %d -> %d", x, y, y + (int)_pos - my));
		int item = getItemIndex(y - my + (int)_pos);
		if (item >= 0 && item < (int)_list.size()) {
			int ybase = getItemY(item);
			//LOG_DEBUG(("%d %d", x - _items_area.x, y - _items_area.y + (int)_pos - ybase));
			if (_list[item]->onMouse(button, pressed, x - _items_area.x, y - _items_area.y + (int)_pos - ybase))
				return true;
	
			if (pressed) {
				if (_current_item != item)
					invalidate(true);
				_current_item = item;
			}
		}
		return true;
	}	
	
	if (_up_area.in(x, y)) {
		if (pressed)
			up();
		return true;
	} else if (_down_area.in(x, y)) {
		if (pressed)
			down();
		return true;
	}
	
	return false;
}

void ScrollList::getSize(int &w, int &h) const {
	w = _background.w; h = _background.h;
}

void ScrollList::remove(const int idx) {
	if (idx < 0 || idx >= (int)_list.size()) {
		return;
	}

	int n = idx;
	List::iterator i;
	for (i = _list.begin(); n--; ++i);
	delete *i;
	_list.erase(i);

	if (_current_item >= (int)_list.size()) 
		_current_item = (int)_list.size() - 1;
	invalidate();
}

void ScrollList::clear() {
	invalidate();
	_current_item = 0;
	for(size_t i = 0; i < _list.size(); ++i) {
		delete _list[i];
	}
	_list.clear();
}

ScrollList::~ScrollList() {
	clear();
}
