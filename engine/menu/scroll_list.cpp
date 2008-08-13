
/* Battle Tanks Game
 * Copyright (C) 2006-2008 Battle Tanks team
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

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
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
#include "menu/textual.h"

ScrollList::ScrollList(const std::string &background, const std::string &font, const int w, const int h, const int spacing, const int hl_h) : 
_client_w(64), _client_h(64), _align(AlignLeft), 
_pos(0), _vel(0), _grab(false), _current_item(0), _spacing(spacing) {
	_background.init(background, w, h, hl_h);
	_font = ResourceManager->loadFont(font, true);
	_scrollers = ResourceManager->loadSurface("menu/v_scroller.png");
}

void ScrollList::set(const int idx) {
	if (idx < 0 || idx >= (int)_list.size())
		throw_ex(("invalid index %d was set", idx));
	if (_current_item != idx) {
		if (_current_item >= 0 && _current_item < (int)_list.size())
			_list[_current_item]->activate(false);
		
		_list[idx]->activate(true);
		_current_item = idx;
		invalidate(true);
	}
}


void ScrollList::initBG(const std::string &background, const int w, const int h, const int hl_h) {
	_background.init(background, w, h, hl_h);
}

const std::string ScrollList::getValue() const { 
	if (_current_item < 0 || _current_item >= (int)_list.size())
		throw_ex(("_current_item is out of range"));
	
	Control *c = _list[_current_item]; 
	TextualControl *l = dynamic_cast<TextualControl *>(c);
	if (l == NULL)
		throw_ex(("cannot getValue from item %d", _current_item));
	return l->get();
}

void ScrollList::append(const std::string &item) {
	append(new Label(_font, item));
}

void ScrollList::append(Control *control) {
	if ((int)_list.size() == _current_item) {
		control->activate(true);
	}
	_list.push_back(control);
	invalidate();
}

void ScrollList::getItemY(const int idx, int &y, int &height) const {
	y = 0;
	int w = 0, h = 0;
	for(int i = 0; i < idx; ++i) {
		_list[i]->get_size(w, h);
		h += _spacing;
		y += h;
	}
	height = h;
}

const int ScrollList::getItemIndex(const int yp) const {
	int y = - _spacing/2;
	for(int i = 0; i < (int)_list.size(); ++i) {
		int w, h;
		_list[i]->get_size(w, h);
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
	int yp = 0, ysize = 0;
	getItemY(_current_item, yp, ysize);
	yp += ysize / 2;
	
	if (_vel != 0) {
		if (math::abs((int)(math::max<int>(yp - _client_h / 2, 0) - _pos)) < 8)
			_vel = 0;
	}

	if (!_grab && (yp < _pos + scroll_marg || yp > _pos + _client_h - scroll_marg)) {
		int dpos = (int)(math::max<int>(yp - _client_h / 2, 0) - _pos);
		_vel = math::max(300, 2 * math::abs(dpos)) * math::sign<int>(dpos);
		_pos += math::sign(dpos) * math::min(math::abs(_vel * dt), math::abs((float)dpos));
	}

	int h = 0, hsize = 0;
	getItemY(_list.size(), h, hsize);
	
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


void ScrollList::render(sdlx::Surface &surface, const int x, const int y) const {
	_background.render(surface, x, y);
	
	sdlx::Rect old_clip;
	surface.get_clip_rect(old_clip);
	
	
	int mx, my;
	_background.getMargins(mx, my);
	
	_client_w = _background.w - mx * 2;
	_client_h = _background.h - my * 2;

// scrollers' area

	int scroller_h = _scrollers->get_height();
	int scroller_w = _scrollers->get_width() / 6;
	
	_up_area = sdlx::Rect(_client_w + my - scroller_w, my, scroller_w, scroller_h);
	surface.blit(*_scrollers, sdlx::Rect(0, 0, scroller_w, scroller_h), x + (int)_up_area.x, y + (int)_up_area.y);
	_down_area = sdlx::Rect(_up_area.x, my + _client_h - scroller_h, scroller_w, scroller_h);
	surface.blit(*_scrollers, sdlx::Rect(scroller_w, 0, scroller_w, scroller_h), x + (int)_down_area.x, y + (int)_down_area.y);
	
	_items_area = sdlx::Rect(mx, my, _client_w - 2 * mx, _client_h);
	_scroller_area = sdlx::Rect(_client_w + my - scroller_w, my, scroller_w, _items_area.h - 2 * scroller_h);

	if (_list.empty()) {
		Container::render(surface, x, y);
		return;
	}
//main list
	
	surface.set_clip_rect(sdlx::Rect(x + mx, y + my, _items_area.w, _items_area.h));

	assert(_client_h > 0);
	//int p = 0;
	int p = getItemIndex((int)_pos);
	//int n = p + (_client_h - 1) / _item_h + 2;
	//if (n > (int)_list.size()) 
	int n = _list.size();
	assert(p>= 0 && p < (int)_list.size());
	
	int item_pos = 0, item_size = 0;
	getItemY(p, item_pos, item_size);
	int yp = my + (_spacing + 1)/ 2 + y - ((int)_pos - item_pos);

	int average_h = 0;
	int visible_items = 0;
	for(; p < n; ++p) {
		int w, h;
		_list[p]->get_size(w, h);
		h += _spacing;
		average_h += h;	
		++visible_items;	

		if (p == (int)_current_item) {
			_background.renderHL(surface, x - 3 * mx, yp + h / 2 - _spacing / 2 + 1);
		}
		//_font->render(surface, x + mx, yp, _list[p]);
		int xp = x;
		switch(_align) {
		case AlignLeft: 
			xp += mx; break;
		case AlignRight: 
			xp += _client_w - mx - w;
		case AlignCenter: 
			xp += (_client_w - 2 * mx - w) / 2 + mx;
		}
		_list[p]->render(surface, xp, yp);
		yp += h;

		if (yp - y - my > _items_area.h) 
			break;
	}

	surface.set_clip_rect(old_clip);

	int boxes = _scroller_area.h / scroller_h;
	average_h /= visible_items;
	int total_h = average_h * n;
	if (visible_items > 0 && boxes > 1 && total_h > _items_area.h) {
		//render scrollers
		int vboxes = boxes * _scroller_area.h / total_h - 2;
		if (vboxes < 0)
			vboxes = 0;
		
		//note total_h > _items_area.h in line below !
		_scroll_mul = 1.0f * (_scroller_area.h - (vboxes + 2) * scroller_h) / (total_h - _items_area.h);
		int scroller_pos = (int)(_pos * _scroll_mul);

		int xp = x + (int)_up_area.x;
		int yp = y + (int)_up_area.y + (int)_up_area.h + scroller_pos;
		surface.blit(*_scrollers, sdlx::Rect(scroller_w * 3, 0, scroller_w, scroller_h), xp, yp);
		yp += scroller_h;
		for(int i = 0; i < vboxes; ++i, yp += scroller_h) {
			surface.blit(*_scrollers, sdlx::Rect(scroller_w * 4, 0, scroller_w, scroller_h), xp, yp);
		}
		surface.blit(*_scrollers, sdlx::Rect(scroller_w * 5, 0, scroller_w, scroller_h), xp, yp);
	}
	
	Container::render(surface, x, y);
}

bool ScrollList::onKey(const SDL_keysym sym) {
	_grab = false;
	if (Container::onKey(sym))
		return true;

	switch(sym.sym) {
	case SDLK_PAGEUP:
		up(10);
		return true;
		
	case SDLK_UP:
		up();
		return true;

	case SDLK_HOME: 
		set(0);
		return true;

	case SDLK_END: 
		set((int)_list.size() - 1);
		return true;

	case SDLK_PAGEDOWN:
		down(10);
		return true;

	case SDLK_DOWN:
		down(1);
		return true;

	default: 
		//LOG_DEBUG(("%d", sym.sym));
		size_t i;
		int c = tolower(sym.sym);
		for(i = 0; i < _list.size(); ++i) {
			TextualControl *l = dynamic_cast<TextualControl *>(_list[(i + _current_item + 1) % _list.size()]);
			if (l != NULL && !l->get().empty()) {
				int fc = tolower(l->get()[0]);
				if (fc == c) 
					break;
			}
		}
		if (i < _list.size()) {
			i = (i + _current_item + 1) % _list.size();
			set(i);
			return true;
		}
		return false;
	}

	return false;
}

void ScrollList::up(const int n) {
	_grab = false;
	if (_list.empty())
		return;
	
	int i = _current_item - n;
	if (i < 0)
		i = 0;
	
	set(i);
}

void ScrollList::down(const int n) {
	_grab = false;
	if (_list.empty())
		return;
	
	int i = _current_item + n;
	if (i >= (int)_list.size())
		i = (int)_list.size() - 1;
	
	set(i);
}

bool ScrollList::onMouse(const int button, const bool pressed, const int x, const int y) {
	if (Container::onMouse(button, pressed, x, y))
		return true;
	//implement dragging of scroller here.
	//LOG_DEBUG(("boo %d %d %d %d", button, pressed, x, y));
	
	if (button == SDL_BUTTON_MIDDLE) //skip accidental wheel clicks
		return false;

	if (button == SDL_BUTTON_WHEELUP) {
		if (!pressed)
			up();
		return true;
	}

	if (button == SDL_BUTTON_WHEELDOWN) {
		if (!pressed)
			down();
		return true;
	}

	int mx, my;
	_background.getMargins(mx, my);
	
	if (_items_area.in(x, y)) {
		_grab = false;
		//LOG_DEBUG(("%d %d -> %d", x, y, y + (int)_pos - my));
		int item = getItemIndex(y - my + (int)_pos);
		if (item >= 0 && item < (int)_list.size()) {
			int ybase = 0, ysize = 0;
			getItemY(item, ybase, ysize);
			//LOG_DEBUG(("%d %d", x - _items_area.x, y - _items_area.y + (int)_pos - ybase));
			if (_list[item]->onMouse(button, pressed, x - _items_area.x, y - _items_area.y + (int)_pos - ybase))
				return true;
	
			if (pressed) {
				set(item);
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

bool ScrollList::onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel) {
	if (state == 0 || _scroll_mul <= 0) {
		//_grab = false;
		return true;
	}
	_grab = true;
	//LOG_DEBUG(("mouse motion %d %d", xrel, yrel));
	_pos += yrel / _scroll_mul;
	return true;
}

void ScrollList::get_size(int &w, int &h) const {
	w = _background.w; h = _background.h;
}

void ScrollList::remove(const int idx) {
	if (idx < 0 || idx >= (int)_list.size()) {
		return;
	}

	int n = idx;
	List::iterator i;
	for (i = _list.begin(); n--; ++i);
	(*i)->activate(false);
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
		_list[i]->activate(false);
		delete _list[i];
	}
	_list.clear();
}

ScrollList::~ScrollList() {
	clear();
}

#include <algorithm>

struct textual_less_eq {
	bool operator()(Control * a, Control * b) const {
		TextualControl * ta = dynamic_cast<TextualControl *>(a);
		TextualControl * tb = dynamic_cast<TextualControl *>(b);
		if (ta == NULL) 
			return true;
		if (tb == NULL)
			return false;
		
		return ta->get() < tb->get();
	}
};

void ScrollList::sort() {
	if (_list.empty())
		return;
	
	if (_current_item >= (int)_list.size())
		_current_item = 0;
	
	Control *selected = _list[_current_item];

	std::sort(_list.begin(), _list.end(), textual_less_eq());

	for(size_t i = 0; i < _list.size(); ++i) {
		if (((Control *)_list[i]) == selected) {
			_current_item = i;
			return;
		}
	}
}

void ScrollList::setHLColor(int r, int g, int b, int a) {
	_background.setHLColor(r, g, b, a);
}

void ScrollList::hide(const bool hide) {
	if (hide && !_hidden && _current_item < (int)_list.size()) {
		_list[_current_item]->activate(false);
	} else if (!hide && _hidden && _current_item < (int)_list.size()) {
		_list[_current_item]->activate(true);
	}
	Control::hide(hide);
}

Control * ScrollList::getItem(const int idx) { 
	if (idx < 0 || idx >= (int)_list.size())
		throw_ex(("invalid index %d", idx));
	
	return _list[idx]; 
}

const Control * ScrollList::getItem(const int idx) const { 
	if (idx < 0 || idx >= (int)_list.size())
		throw_ex(("invalid index %d", idx));

	return _list[idx]; 
}

const int ScrollList::get() const {
	if (_current_item >= (int)_list.size())
		throw_ex(("get(): invalid internal index %d/%d", _current_item, (int)_list.size()));
	return _current_item; 
}
