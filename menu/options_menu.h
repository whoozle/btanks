#ifndef BTANKS_OPTIONS_MENU_H__
#define BTANKS_OPTIONS_MENU_H__

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

#include "sdlx/rect.h"
#include "base_menu.h"
#include "box.h"
#include "alarm.h"


class Button;
class Slider;
class MainMenu;
class ControlPicker;
class Object;
class RedefineKeys;
class GamepadSetup;
class Chooser;
class Checkbox;

class OptionsMenu : public BaseMenu {
public:
	OptionsMenu(MainMenu *parent, const int w, const int h);
	~OptionsMenu();
	
	void getSize(int &w, int &h) const;
	void tick(const float dt);
	
	void reload();
	void save();
	
	virtual bool onKey(const SDL_keysym sym);
	virtual void render(sdlx::Surface &dst, const int x, const int y);

private: 
	MainMenu *_parent;
	ControlPicker *sp, *sp1, *sp2;

	Box _background;
	int _bx, _by;
	Button *_b_ok, *_b_back;
	Slider *_fx, *_music;
	Button *_b_redefine;
	Chooser *_c_res;
	Checkbox *_fsmode, *_donate;

	Alarm _shoot;
	
	RedefineKeys * _keys;
	GamepadSetup * _gamepad;
};

#endif

