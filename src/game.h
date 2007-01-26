#ifndef __BT_GAME_H__
#define __BT_GAME_H__

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

#include "mrt/singleton.h"
#include <string>
#include <vector>
#include <sigc++/sigc++.h>

#include "menu/menu.h"
#include "math/v3.h"
#include "player_state.h"
#include "alarm.h"
#include "window.h"

class BaseObject;
class Object;
class Message;
class Server;
class Client;
class Connection;
class ControlMethod;
class PlayerSlot;
class Hud;
class Credits;
class Cheater;
class Console;


class IGame : public Window {

class marshaller {
public: 
	typedef bool result_type;

	template<typename IteratorT>
    	bool operator()(IteratorT First, IteratorT Last) {
    		while(First != Last) {
    			if (*First) {
    				return true;
    			}
    			++First;
    		}
    		return false;
    	}
};

public: 
	DECLARE_SINGLETON(IGame);


	static const std::string data_dir;
	//signals
	sigc::signal1<bool, const SDL_keysym, marshaller> key_signal;
	sigc::signal4<void, const int, const bool, const int, const int> mouse_signal;

	void init(const int argc, char *argv[]);
	void run();
	void deinit();
	
	void clear();


	IGame();
	~IGame();
	
	//stupid visual effect
	void shake(const float duration, const int intensity);
	
	void resetLoadingBar(const int total);
	void notifyLoadingBar(const int progress = 1);

	void loadMap(const std::string &name, const bool spawn = true);	
	void setMyIndex(const int idx) { _my_index = idx; }
	
	const std::string getRandomWaypoint(const std::string &classname, const std::string &last_wp = std::string()) const;
	const std::string getNearestWaypoint(const BaseObject *obj, const std::string &classname) const;
	void getWaypoint(v3<float> &wp, const std::string &classname, const std::string &name);

private:
	bool onKey(const SDL_keysym sym);
	void onMenu(const std::string &name, const std::string &value);
	
	void stopCredits();

	bool _running, _paused;

	MainMenu _main_menu;
	
	struct Item {
		int id;
		std::string classname, animation;
		v3<int> position;
		Uint32 dead_on;
	};
	typedef std::deque<Item> Items;
	Items _items;
	Alarm _check_items;
	void checkItems();
	
	typedef std::map<const std::string, v3<int> > WaypointMap;
	typedef std::map<const std::string, WaypointMap> WaypointClassMap;
	typedef std::multimap<const std::string, std::string> WaypointEdgeMap;
	
	WaypointClassMap _waypoints;
	WaypointEdgeMap  _waypoint_edges;
	
	bool _show_fps;
	Object *_fps;

	std::string _preload_map;
	bool _autojoin;

	float _shake;
	int _shake_int;
	int _my_index;
	
	Hud *_hud;
	int _loading_bar_total, _loading_bar_now;
	Uint32 t_start;
	
	Credits *_credits;
	Cheater *_cheater;
	Console *_console;
	
	IGame(const IGame &);
	const IGame& operator=(const IGame &);
};

SINGLETON(Game, IGame);

#endif

