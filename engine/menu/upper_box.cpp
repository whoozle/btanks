
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
#include "upper_box.h"
#include "resource_manager.h"
#include "config.h"
#include "sdlx/surface.h"
#include "sdlx/font.h"
#include "i18n.h"
#include "box.h"
#include "player_name_control.h"
#include "prompt.h"
#include "text_control.h"

void UpperBox::update(const GameType game_type) {
	switch(game_type) {
	case GameTypeDeathMatch:
		value = "deathmatch"; break;
	case GameTypeCooperative:
		value = "cooperative"; break;
	case GameTypeRacing:
		value = "racing"; break;
	default: 
		throw_ex(("invalid game_type value! (%d)", (int)game_type));
	}
}

UpperBox::UpperBox(int _w, int _h, const bool server) : value("deathmatch"), _server(server), _checkbox(NULL) {
	add(0, 0, _box = new Box("menu/background_box.png", _w, _h));
	
	int mx, my;
	_box->getMargins(mx, my);
	
	_medium = ResourceManager->loadFont("medium", true);
	_big = ResourceManager->loadFont("big", true);
	
	int w, h;
	get_size(w, h);
	
	int players_w = w / 5;
	
	int cw1, ch1, cw2, ch2;
	_player1_name = new PlayerNameControl(I18n->get("menu", "player-name-1"), "player.name-1", players_w);
	_player1_name->get_size(cw1, ch1);

	_player2_name = new PlayerNameControl(I18n->get("menu", "player-name-2"), "player.name-2", players_w);
	_player2_name->get_size(cw2, ch2);

	const int dh = 8;
	add(w - players_w - mx, my + (h - (ch1 + ch2) - dh) / 2 - ch1, _player1_name);
	add(w - players_w - mx, my + (h - (ch1 + ch2) + dh) / 2, _player2_name);

	_name_prompt = new Prompt(320, 80, new TextControl("small", 32));
	int nw, nh;
	get_size(w, h);
	_name_prompt->get_size(nw, nh);
	add(w - nw, (h - nh) / 2, _name_prompt);
	_name_prompt->hide();
}

void UpperBox::render(sdlx::Surface &surface, const int x, const int y) const{
	AUTOLOAD_SURFACE(_checkbox, "menu/radio.png");

	Container::render(surface, x, y);
	
	int font_dy = (_big->get_height() - _medium->get_height()) / 2;
	
	int wt = 0;
	int line1_y = 10;
	wt = _big->render(surface, x + 16, y + line1_y, I18n->get("menu", "mode"));
	
	int line2_y = 40;
	
	int wt2 = _big->render(surface, x + 16, y + line2_y, I18n->get("menu", "split-screen"));
	if (wt2 > wt)
		wt = wt2;

	wt += 48;

	_medium->render(surface, x + wt, y + line1_y + font_dy, I18n->get("menu/modes", value));
	
	int cw = _checkbox->get_width() / 2;
	
	sdlx::Rect off(0, 0, cw, _checkbox->get_height());
	sdlx::Rect on(cw, 0, _checkbox->get_width(), _checkbox->get_height());
	
	bool split;
	Config->get("multiplayer.split-screen-mode", split, false);
	
	_off_area.x = wt;
	_off_area.y = line2_y;
	_off_area.w = wt;
	_on_area.h = _off_area.h = 32;
	
	surface.blit(*_checkbox, split?off:on, x + wt, y + line2_y + font_dy);
	wt += cw;
	wt += 16 + _medium->render(surface, x + wt, y + line2_y + font_dy - 2, I18n->get("menu", "off"));
	_off_area.w = wt - _off_area.w + 1;

	_on_area.x = wt;
	_on_area.y = line2_y;
	_on_area.w = wt;
	surface.blit(*_checkbox, split?on:off, x + wt, y + line2_y + font_dy);
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
	bool split;
	
	Config->get("multiplayer.split-screen-mode", split, false);
	if (split) {
		if (_player2_name->hidden())
			_player2_name->hide(false);
	} else {
		if (!_player2_name->hidden())
			_player2_name->hide(true);
	}
	
	if (_player1_name->changed()) {
		_player1_name->reset();
		if (_player1_name->edit()) {
			_edit_player1 = true;
			_name_prompt->hide(false);
			_name_prompt->set(_player1_name->get());
			_name_prompt->reset();
		} 
	}
	
	//copypasteninja was here.
	if (_player2_name->changed()) {
		_player2_name->reset();
		if (_player2_name->edit()) {
			_edit_player1 = false;
			_name_prompt->hide(false);
			_name_prompt->set(_player2_name->get());
			_name_prompt->reset();
		} 
	}
	
	if (_name_prompt->changed()) {
		_name_prompt->reset();
		_name_prompt->hide();
		std::string name = _name_prompt->get();
		if (!name.empty()) {
			LOG_DEBUG(("setting name to %s", name.c_str()));
			(_edit_player1?_player1_name:_player2_name)->set(name);
		}
	}
}
