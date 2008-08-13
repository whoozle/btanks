
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
#include "container.h"
#include "mrt/logger.h"
#include "sdlx/rect.h"

void Container::tick(const float dt) {
	for(ControlList::iterator i = _controls.begin(); i != _controls.end(); ++i) {
		if ((*i)->hidden())
			continue;
		
		(*i)->tick(dt);
	}
}


void Container::render(sdlx::Surface &surface, const int x, const int y) const {
	for(ControlList::const_iterator i = _controls.begin(); i != _controls.end(); ++i) {
		Control *c = *i;
		if (c->hidden())
			continue;

		int base_x, base_y;
		c->get_base(base_x, base_y);
		c->render(surface, x + base_x, y + base_y);
	}
}

void Container::get_size(int &w, int &h) const {
	w = h = 0;
	for(ControlList::const_iterator i = _controls.begin(); i != _controls.end(); ++i) {
		int cw = -1, ch = -1; //for a broken controls
		(*i)->get_size(cw, ch);
		assert(cw != -1 && ch != -1);

		int x2, y2;
		(*i)->get_base(x2, y2);
		
		x2 += cw;
		y2 += ch;

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
		if ((*i)->hidden() || (*i) == _focus)
			continue;

		if ((*i)->onKey(sym))
			return true;
	}
	return false;
}

bool Container::onMouse(const int button, const bool pressed, const int x, const int y) {
	//LOG_DEBUG(("%p: entering onMouse handler. (%d, %d)", (void *)this, x , y));
	for(ControlList::reverse_iterator i = _controls.rbegin(); i != _controls.rend(); ++i) {
		if ((*i)->hidden())
			continue;
		int bw, bh, base_x, base_y;
		(*i)->get_size(bw, bh);
		(*i)->get_base(base_x, base_y);
		
		const sdlx::Rect dst(base_x, base_y, bw, bh);
		//LOG_DEBUG(("%p: checking control %p (%d, %d, %d, %d)", (void *)this, (void *)(*i), dst.x, dst.y, dst.w, dst.h));
		if (dst.in(x, y)) {
			if (pressed) {
				//LOG_DEBUG(("%p: focus passed to %p", (void *)this,  (void *)_focus));
				_focus = (*i);
			}
			if ((*i)->onMouse(button, pressed, x - dst.x, y - dst.y)) {
				//LOG_DEBUG(("%p: control %p returning true", (void *)this, (void *)(*i)));
				return true;
			}
		}
	}
	return false;
}

bool Container::onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel) {
	for(ControlList::reverse_iterator i = _controls.rbegin(); i != _controls.rend(); ++i) {
		Control *c = (*i);
		if (c->hidden())
			continue;
		int bw, bh, base_x, base_y;
		c->get_size(bw, bh);
		c->get_base(base_x, base_y);
		
		const sdlx::Rect dst(base_x, base_y, bw, bh);
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
	ctrl->set_base(x, y);
#ifdef DEBUG
	for(ControlList::iterator i = _controls.begin(); i != _controls.end(); ++i) {
		assert(ctrl != (*i)); //double add! 
	}
#endif
	_controls.push_back(ctrl);
}

Container::~Container() {
	clear();
}

void Container::clear() {
	for(ControlList::iterator i = _controls.begin(); i != _controls.end(); ++i) {
		delete (*i);
	}
	_controls.clear();
	_focus = NULL;
}

const bool Container::in(const Control *c, const int x, const int y) const {
	assert(c != NULL);
	ControlList::const_reverse_iterator i;
	for(i = _controls.rbegin(); i != _controls.rend(); ++i) {
		if ((*i) == c)
			break;
	}
	if (i == _controls.rend())
		throw_ex(("no control %p in container %p", (const void *)c, (const void *)this));
	
	int bw, bh, base_x, base_y;
	c->get_size(bw, bh);
	c->get_base(base_x, base_y);
	
	const sdlx::Rect dst(base_x, base_y, bw, bh);
	return dst.in(x, y);
}

void Container::activate(const bool active) {
/*	int x, y;
	sdlx::Cursor::get_position(x, y);
	for(ControlList::reverse_iterator i = _controls.rbegin(); i != _controls.rend(); ++i) {
		Control *c = (*i);
		if (c->hidden())
			continue;
		if (active) {
			int bw, bh;
			c->get_size(bw, bh);
			const sdlx::Rect dst(i->first.x, i->first.y, bw, bh);
			bool in = dst.in(x, y);
		
		} else {
			if (c->_mouse_in) {
				c->_mouse_in = false;
				c->on_mouse_enter(false);
			}
		}
		if (active && !c->_mouse_in) {
			c->_mouse_in = true;
			c->on_mouse_enter(true);
		} else if (!in && c->_mouse_in) {
			c->_mouse_in = false;
			c->on_mouse_enter(false);
		}
	}
*/
}
