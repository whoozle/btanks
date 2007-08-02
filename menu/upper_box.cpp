
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
#include "i18n.h"
#include "box.h"
#include "player_name_control.h"

UpperBox::UpperBox(int _w, int _h, const bool server) : _server(server) {
	_checkbox = ResourceManager->loadSurface("menu/radio.png");
	Config->get("multiplayer.game-type", value, "deathmatch");
	add(0, 0, _box = new Box("menu/background_box.png", _w, _h));
	
	int mx, my;
	_box->getMargins(mx, my);
	
	_medium = ResourceManager->loadFont("medium", true);
	_big = ResourceManager->loadFont("big", true);
	
	int w, h;
	getSize(w, h);
	
	int cw1, ch1, cw2, ch2;
	_player1_name = new PlayerNameControl(I18n->get("menu", "player-name-1"), "player.name-1");
	_player1_name->getSize(cw1, ch1);

	_player2_name = new PlayerNameControl(I18n->get("menu", "player-name-2"), "player.name-2");
	_player2_name->getSize(cw2, ch2);

	const int dh = 8;
	add(w - cw1 - 2 * mx, my + (h - (ch1 + ch2) - dh) / 2 - ch1, _player1_name);
	add(w - cw2 - 2 * mx, my + (h - (ch1 + ch2) + dh) / 2, _player2_name);
}

void UpperBox::layout() {
	int mx, my;
	_box->getMargins(mx, my);

	int w, h;
	getSize(w, h);
	
	int cw1, ch1, cw2, ch2;
	_player1_name->getSize(cw1, ch1);
	_player2_name->getSize(cw2, ch2);

	const int dh = 8;
	setBase(_player1_name, w - cw1 - 2 * mx, my + (h - (ch1 + ch2) - dh) / 2 - ch1);
	setBase(_player2_name, w - cw2 - 2 * mx, my + (h - (ch1 + ch2) + dh) / 2);
}

void UpperBox::render(sdlx::Surface &surface, const int x, const int y) {
	Container::render(surface, x, y);
	
	int font_dy = (_big->getHeight() - _medium->getHeight()) / 2;
	
	int wt = 0;
	int line1_y = 10;
	wt = _big->render(surface, x + 16, y + line1_y, I18n->get("menu", "mode"));
	
	int line2_y = 40;
	
	int wt2 = _big->render(surface, x + 16, y + line2_y, I18n->get("menu", "split-screen"));
	if (wt2 > wt)
		wt = wt2;

	wt += 48;

	_medium->render(surface, x + wt, y + line1_y + font_dy, I18n->get("menu/modes", value));
	
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
	wt += 16 + _medium->render(surface, x + wt, y + line2_y + font_dy - 2, I18n->get("menu", "off"));
	_off_area.w = wt - _off_area.w + 1;

	_on_area.x = wt;
	_on_area.y = line2_y;
	_on_area.w = wt;
	surface.copyFrom(*_checkbox, split?on:off, x + wt, y + line2_y);
	wt += cw;
	wt += 16 + _medium->render(surface, x + wt, y + line2_y + font_dy - 2, I18n->get("menu", "on"));
	_on_area.w = wt - _on_area.w + 1;
}

bool UpperBox::onMouse(const int button, const bool pressed, const int x, const int y) {
	if (Container::onMouse(button, pressed, x, y))
		return true;
	
	if (!pressed) 
		return false;
	
	if (_on_area.in(x, y)) {
		//LOG_DEBUG(("split screen on!"));
		Config->set("multiplayer.split-screen-mode", true);
		invalidate();
		return true;
	} else if (_off_area.in(x, y)) {
		//LOG_DEBUG(("split screen off!"));
		Config->set("multiplayer.split-screen-mode", false);
		invalidate();
		return true;
	}
	return false;
}

void UpperBox::tick(const float dt) {
	Container::tick(dt);
	bool layout = false;
	if (_player1_name->changed()) {
		_player1_name->reset();
		layout = true;
	}
	if (_player2_name->changed()) {
		layout = true;
		_player2_name->reset();
	}
	if (layout)
		this->layout();
}
