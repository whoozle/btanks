
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
#include "label.h"
#include "sdlx/font.h"
#include "resource_manager.h"

Label::Label(const sdlx::Font *font, const std::string &label) : _font(font), _label(label) {}
Label::Label(const std::string &font, const std::string &label) : _font(ResourceManager->loadFont(font, true)), _label(label) {}

void Label::getSize(int &w, int &h) const {
	w = _font->render(NULL, 0, 0, _label);
	h = _font->getHeight();
}

void Label::setFont(const std::string &font) {
	_font = ResourceManager->loadFont(font, true);
}

void Label::set(const std::string &label) {
	_label = label;
}

const std::string Label::get() const { 
	return _label; 
}

void Label::render(sdlx::Surface& surface, int x, int y) {
	_font->render(surface, x, y, _label);
}
