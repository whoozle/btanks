#ifndef __BT_GAME_H__
#define __BT_GAME_H__
/* Battle Tanks Game
 * Copyright (C) 2006 Battle Tanks team
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

#include "sdlx/surface.h"
#include "mrt/singleton.h"
#include <string>
#include <vector>
#include <sigc++/sigc++.h>

#include "menu.h"
#include "tmx/map.h"

class Object;

class IGame {
public: 
	DECLARE_SINGLETON(IGame);

	static const std::string data_dir;
	//signals
	sigc::signal2<void, const Uint8, const SDL_keysym> key_signal;

	void init(const int argv, const char **argc);
	void run();
	void deinit();


	IGame();
	~IGame();
private:
	void onKey(const Uint8 type, const SDL_keysym sym);
	void onMenu(const std::string &name);

	bool _running, _paused;
	sdlx::Surface _window;

	MainMenu _main_menu;
	Map _map;
	
	std::vector<Object *> _players;
};

SINGLETON(Game, IGame);

#endif

