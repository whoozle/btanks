
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
#include "upper_box.h"
#include "resource_manager.h"
#include "config.h"
#include "sdlx/surface.h"
#include "sdlx/font.h"


UpperBox::UpperBox(int w, int h, const bool server): _w(w), _h(h), _server(server) {
	_checkbox = ResourceManager->loadSurface("menu/radio.png");
	Config->get("multiplayer.game-type", value, "deathmatch");
	Box::init("menu/background_box.png", w, h);

	_medium = ResourceManager->loadFont("medium", true);
	_big = ResourceManager->loadFont("big", true);
}

void UpperBox::getSize(int &w, int &h) const {
	w = _w; h = _h;
}


void UpperBox::render(sdlx::Surface &surface, const int x, const int y) {
	Box::render(surface, x, y);
	
	int font_dy = (_big->getHeight() - _medium->getHeight()) / 2;
	
	int wt = 0;
	wt = _big->render(surface, x + 16, y + 16, "MODE");
	_medium->render(surface, x + (w - wt - 32) / 2, y + 16 + font_dy, value);
	
	int line2_y = 46;
	
	wt = _big->render(surface, x + 16, y + line2_y, "SPLIT SCREEN");
	wt += 96;
	
	int cw = _checkbox->getWidth() / 2;
	
	sdlx::Rect off(0, 0, cw, _checkbox->getHeight());
	sdlx::Rect on(cw, 0, _checkbox->getWidth(), _checkbox->getHeight());
	
	bool split;
	Config->get("multiplayer.split-screen-mode", split, false);
	
	_off_area.x = wt;
	_off_area.y = line2_y;
	_off_area.w = wt;
	_on_area.h = _off_area.h = 32;
	
	surface.copyFrom(*_checkbox, split?off:on, x + wt, y + line2_y);
	wt += cw;
	wt += 16 + _medium->render(surface, x + wt, y + line2_y + font_dy - 2, "OFF");
	_off_area.w = wt - _off_area.w + 1;

	_on_area.x = wt;
	_on_area.y = line2_y;
	_on_area.w = wt;
	surface.copyFrom(*_checkbox, split?on:off, x + wt, y + line2_y);
	wt += cw;
	wt += 16 + _medium->render(surface, x + wt, y + line2_y + font_dy - 2, "ON");
	_on_area.w = wt - _on_area.w + 1;
}

bool UpperBox::onMouse(const int button, const bool pressed, const int x, const int y) {
	if (!pressed) 
		return false;
	
	if (_on_area.in(x, y)) {
		//LOG_DEBUG(("split screen on!"));
		Config->set("multiplayer.split-screen-mode", true);
		_changed = true;
		return true;
	} else if (_off_area.in(x, y)) {
		//LOG_DEBUG(("split screen off!"));
		Config->set("multiplayer.split-screen-mode", false);
		_changed = true;
		return true;
	}
	return false;
}
