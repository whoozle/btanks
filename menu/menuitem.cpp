
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

#include "menuitem.h"
#include "sdlx/font.h"
#include "mrt/logger.h"
#include "math/binary.h"

MenuItem::MenuItem(const sdlx::Font *font, const std::string &name, const std::string &type, const std::string &text, const std::string &value) : 
	name(name), type(type), 
	_text(text), _value(value),
	_font(font)
{
	render();
}

const std::string MenuItem::getValue() const {
	return _value;
}


void MenuItem::render() {
	_normal.free();

	_font->render(_normal, (_text.empty())?" ":_text);
	_normal.convertAlpha();
	_normal.convertToHardware();
}
	
void MenuItem::render(sdlx::Surface &dst, const int x, const int y) {
	dst.copyFrom(_normal, x, y);
}

void MenuItem::getSize(int &w, int &h) const {
	w = _normal.getWidth();
	h = _normal.getHeight();
}

const bool MenuItem::onKey(const SDL_keysym sym) {
	return false;
}

void MenuItem::onFocus() {}
void MenuItem::onLeave() {}
