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
#include "math/v3.h"

class Object;
class PlayerState;
class Message;
class Server;
class Client;
class Connection;

class IGame {
public: 
	DECLARE_SINGLETON(IGame);

	static const std::string data_dir;
	//signals
	sigc::signal2<void, const Uint8, const SDL_keysym> key_signal;

	void init(const int argc, char *argv[]);
	void run();
	void deinit();


	IGame();
	~IGame();
	
	//multiplayer stuff. refactor it.
	void notify(const PlayerState& state);
	void onClient(Message &message);
	void onMessage(const Connection &connection, const Message &message);
	
	const Map &getMap() const { return _map; }
private:
	void onKey(const Uint8 type, const SDL_keysym sym);
	void onMenu(const std::string &name);

	bool _running, _paused;
	sdlx::Surface _window;

	MainMenu _main_menu;
	Map _map;
	
	void loadMap(const std::string &name);	
	const int spawnPlayer(const std::string &classname, const std::string &animation);
	struct PlayerSlot {
		PlayerSlot() : obj(NULL) {}
		PlayerSlot(const Object *obj) : obj(obj) {}
		const Object * obj;
		v3<int> position;
	};
	
	std::vector<PlayerSlot> _players;
	int _my_index;
	
	Server *_server;
	Client *_client;
};

SINGLETON(Game, IGame);

#endif

