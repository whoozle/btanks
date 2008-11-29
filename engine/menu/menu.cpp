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

#include "menu.h"
#include "menu_item.h"
#include "sound/mixer.h"
#include "sdlx/font.h"

Menu::Menu() : spacing(4), current_item(0),  width(0), height(0) {}

void Menu::add(MenuItem *item) {
	int w;
	get_size(w, height);
	int cw, ch;
	item->get_size(cw, ch);
	
	if (w >= cw) {
		Container::add((w - cw) / 2, height + spacing, item);
	} else {
		int dx = (cw - w) / 2;
		for(ControlList::iterator i = _controls.begin(); i != _controls.end(); ++i) {
			int bx, by;
			(*i)->get_base(bx, by);
			(*i)->set_base(bx + dx, by);
		}
		Container::add(0, height + spacing, item);
	}
	get_size(width, height);
}

const Control * Menu::get_current_item() const {
	int idx = 0;
	for(ControlList::const_iterator i = _controls.begin(); i != _controls.end(); ++i) {
		if (idx++ == current_item)
			return *i;
	}
	return NULL;
}

Control * Menu::get_current_item() {
	int idx = 0;
	for(ControlList::iterator i = _controls.begin(); i != _controls.end(); ++i) {
		if (idx++ == current_item)
			return *i;
	}
	return NULL;
}


void Menu::render(sdlx::Surface &surface, const int x, const int y) const {
	int bw, bh;
	background.get_size(bw, bh);
	int dx = (width - bw) / 2, dy = (height - bh) / 2;
	background.render(surface, x + dx, y + dy);
	
	const Control *c = get_current_item();
	int cx, cy;
	c->get_base(cx, cy);
	int cw, ch;
	const MenuItem * mi = dynamic_cast<const MenuItem *>(c);
	if (mi != NULL) {
		//optimization :)
		ch = mi->get_font()->get_height();
	} else {
		c->get_size(cw, ch);
	}
	
	background.renderHL(surface, x + dx, y + cy + ch / 2 + 2);
	
	Container::render(surface, x, y);
}

bool Menu::onKey(const SDL_keysym sym) {
	Control *item = get_current_item();
	if (item != NULL && item->onKey(sym)) {
		if (item->changed()) {
			item->reset();
			invalidate();
		}
	}
	switch(sym.sym) {
	case SDLK_UP: 
		up();
		return true;
	case SDLK_DOWN: 
		down();
		return true;
	case SDLK_ESCAPE: 
		hide();
		return true;
	default: 
		return false;
	}
}

void Menu::up() {
	int n = _controls.size();
	--current_item;
	if (current_item < 0)
		current_item += n;

	Mixer->playSample(NULL, "menu/move.ogg", false);
}

void Menu::down() {
	int n = _controls.size();
	++current_item;
	if (current_item >= n) 
		current_item %= n;

	Mixer->playSample(NULL, "menu/move.ogg", false);
}


bool Menu::onMouse(const int button, const bool pressed, const int x, const int y) {
	if (Container::onMouse(button, pressed, x, y)) {
		int idx;
		for(ControlList::const_iterator i = _controls.begin(); i != _controls.end(); ++i, ++idx) {
			Control * c = *i;
			if (c->changed()) {
				c->reset();
				current_item = idx;
				invalidate();
			}
		}
		return true;
	}
	return false;
}

bool Menu::onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel) {
	return Container::onMouseMotion(state, x, y, xrel, yrel);
}
