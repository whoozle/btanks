
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
#include "button.h"
#include "sdlx/font.h"
#include "resource_manager.h"

Button::Button(const std::string &font, const std::string &label) : _font(ResourceManager->loadFont(font, true)), _label(label) {
	_w = _font->render(NULL, 0, 0, label);
	_background.init("menu/background_box.png", _w + 24, _font->get_height() + 8);
}


void Button::render(sdlx::Surface& surface, int x, int y) const {
	_background.render(surface, x, y);
	
	_font->render(surface, x + (_background.w - _w) / 2, y + (_background.h - _font->get_height()) / 2, _label);
}

bool Button::onMouse(const int button, const bool pressed, const int x, const int y) {
	if (pressed) 
		return true;
	invalidate(true);
	return true;
}

void Button::get_size(int &w, int &h) const {
	w = _background.w;
	h = _background.h;
}

void Button::on_mouse_enter(bool enter) {
	if (enter && _background.get_background() == "menu/background_box.png") {
		_background.set_background("menu/background_box_dark.png");
	} else if (!enter && _background.get_background() != "menu/background_box.png") {
		_background.set_background("menu/background_box.png");
	}
}
