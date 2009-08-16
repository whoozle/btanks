#ifndef BTANKS_MENU_MAIN_MENU_DESC_H__
#define BTANKS_MENU_MAIN_MENU_DESC_H__

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
#include "sl08/sl08.h"

class NetworkStatusControl;
class Prompt;

class MainMenu : public Menu {
public: 
	sl08::signal1<void, const std::string &, MainMenu> menu_signal; //backward compatibility, remove later


	MainMenu(int w, int h);
	void init();
	static bool generate_key_events_for_gamepad;
	virtual void tick(const float dt);

	void add(MenuItem *item, Control *slave = NULL);

	virtual bool onKey(const SDL_keysym sym);
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);
	virtual bool onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel);
	virtual void on_mouse_enter(bool enter = true);

	virtual void render(sdlx::Surface &surface, const int x, const int y) const;
	~MainMenu();

	void onEvent(const SDL_Event &e);
	virtual void hide(const bool hide = true);

private: 
	std::vector<Control *> items;
	Control * active;
	
	int w, h, dx, dy;
	NetworkStatusControl * _netstat;
	Prompt * _profile_dialog;
	
	//keyboard emulation 
	bool _key_active;
	int value[2];
};

#endif
