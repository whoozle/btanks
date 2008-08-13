
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
}
	
void MenuItem::render(sdlx::Surface &dst, const int x, const int y) const {
	dst.blit(_normal, x, y);
}

void MenuItem::get_size(int &w, int &h) const {
	w = _normal.get_width();
	h = _normal.get_height();
}

const bool MenuItem::onKey(const SDL_keysym sym) {
	return false;
}

void MenuItem::onFocus() {}
void MenuItem::onLeave() {}
