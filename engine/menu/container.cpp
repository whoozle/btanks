
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
#include "container.h"
#include "mrt/logger.h"
#include "sdlx/rect.h"

void Container::tick(const float dt) {
	for(ControlList::iterator i = _controls.begin(); i != _controls.end(); ++i) {
		if (i->second->hidden())
			continue;
		
		i->second->tick(dt);
	}
}


void Container::render(sdlx::Surface &surface, const int x, const int y) const {
	for(ControlList::const_iterator i = _controls.begin(); i != _controls.end(); ++i) {
		if (i->second->hidden())
			continue;

		const v2<int> &dst = i->first;
		i->second->render(surface, x + dst.x, y + dst.y);
	}
}

void Container::get_size(int &w, int &h) const {
	w = h = 0;
	for(ControlList::const_iterator i = _controls.begin(); i != _controls.end(); ++i) {
		int cw = -1, ch = -1; //for a broken controls
		i->second->get_size(cw, ch);
		assert(cw != -1 && ch != -1);

		int x2 = i->first.x + cw;
		int y2 = i->first.y + ch;

		if (x2 > w) 
			w = x2;

		if (y2 > h)
			h = y2;
	}
}


bool Container::onKey(const SDL_keysym sym) {
	if (_focus != NULL && !_focus->hidden() && _focus->onKey(sym)) //first, pass key event to control with focus
		return true;
	
	for(ControlList::reverse_iterator i = _controls.rbegin(); i != _controls.rend(); ++i) {
		if (i->second->hidden() || i->second == _focus)
			continue;

		if (i->second->onKey(sym))
			return true;
	}
	return false;
}

bool Container::onMouse(const int button, const bool pressed, const int x, const int y) {
	//LOG_DEBUG(("%p: entering onMouse handler. (%d, %d)", (void *)this, x , y));
	for(ControlList::reverse_iterator i = _controls.rbegin(); i != _controls.rend(); ++i) {
		if (i->second->hidden())
			continue;
		int bw, bh;
		i->second->get_size(bw, bh);
		
		const sdlx::Rect dst(i->first.x, i->first.y, bw, bh);
		//LOG_DEBUG(("%p: checking control %p (%d, %d, %d, %d)", (void *)this, (void *)i->second, dst.x, dst.y, dst.w, dst.h));
		if (dst.in(x, y)) {
			if (pressed) {
				//LOG_DEBUG(("%p: focus passed to %p", (void *)this,  (void *)_focus));
				_focus = i->second;
			}
			if (i->second->onMouse(button, pressed, x - dst.x, y - dst.y)) {
				//LOG_DEBUG(("%p: control %p returning true", (void *)this, (void *)i->second));
				return true;
			}
		}
	}
	return false;
}

bool Container::onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel) {
	for(ControlList::reverse_iterator i = _controls.rbegin(); i != _controls.rend(); ++i) {
		Control *c = i->second;
		if (c->hidden())
			continue;
		int bw, bh;
		c->get_size(bw, bh);
		
		const sdlx::Rect dst(i->first.x, i->first.y, bw, bh);
		bool in = dst.in(x, y);
		if (in && !c->_mouse_in) {
			c->_mouse_in = true;
			c->on_mouse_enter(true);
		} else if (!in && c->_mouse_in) {
			c->_mouse_in = false;
			c->on_mouse_enter(false);
		}
		if (in && c->onMouseMotion(state, x - dst.x, y - dst.y, xrel, yrel)) {
			return true;
		}
	}
	return false;
}


void Container::add(const int x, const int y, Control *ctrl) {
	assert(ctrl != NULL);
#ifdef DEBUG
	for(ControlList::iterator i = _controls.begin(); i != _controls.end(); ++i) {
		assert(ctrl != i->second); //double add! 
	}
#endif
	_controls.push_back(ControlList::value_type(v2<int>(x, y), ctrl));
}

Container::~Container() {
	clear();
}

void Container::clear() {
	for(ControlList::iterator i = _controls.begin(); i != _controls.end(); ++i) {
		delete i->second;
	}
	_controls.clear();
	_focus = NULL;
}

const bool Container::in(const Control *c, const int x, const int y) const {
	assert(c != NULL);
	ControlList::const_reverse_iterator i;
	for(i = _controls.rbegin(); i != _controls.rend(); ++i) {
		if (i->second == c)
			break;
	}
	if (i == _controls.rend())
		throw_ex(("no control %p in container %p", (const void *)c, (const void *)this));
	
	int bw, bh;
	c->get_size(bw, bh);
	
	const sdlx::Rect dst(i->first.x, i->first.y, bw, bh);
	return dst.in(x, y);
}

void Container::getBase(const Control *c, int &x, int &y) const {
	assert(c != NULL);
	ControlList::const_reverse_iterator i;
	for(i = _controls.rbegin(); i != _controls.rend(); ++i) {
		if (i->second == c)
			break;
	}
	if (i == _controls.rend())
		throw_ex(("no control %p in container %p", (const void *)c, (const void *)this));

	x = i->first.x; y = i->first.y;	
}

void Container::setBase(const Control *c, const int x, const int y) {
	assert(c != NULL);
	ControlList::reverse_iterator i;
	for(i = _controls.rbegin(); i != _controls.rend(); ++i) {
		if (i->second == c)
			break;
	}
	if (i == _controls.rend())
		throw_ex(("no control %p in container %p", (const void *)c, (const void *)this));

	i->first.x = x; i->first.y = y;	
}
