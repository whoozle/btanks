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
#include <map>
#include <vector>
#include <sigc++/sigc++.h>

#include "math/v2.h"
#include "player_state.h"
#include "alarm.h"
#include "window.h"

#include <sdlx/font.h>
#include "sdlx/timer.h"

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
class MainMenu;

class IGame : public sigc::trackable, public Window {

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

	//signals
	sigc::signal1<bool, const SDL_keysym, marshaller> key_signal;
	sigc::signal4<bool, const int, const bool, const int, const int, marshaller> mouse_signal;

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
	
	const std::string getRandomWaypoint(const std::string &classname, const std::string &last_wp = std::string()) const;
	const std::string getNearestWaypoint(const BaseObject *obj, const std::string &classname) const;
	void getWaypoint(v2<float> &wp, const std::string &classname, const std::string &name);
	
	void gameOver(const std::string &state, const float time);
	void displayMessage(const std::string &message, const float time);

	void resetTimer();

private:
	void pushState(const std::string &state, const float time);
	const std::string popState(const float dt);


	bool onKey(const SDL_keysym sym);
	void onMenu(const std::string &name, const std::string &value);
	const std::string onConsole(const std::string &cmd, const std::string &param);
	
	void stopCredits();

	bool _running, _paused, _map_loaded, _game_over;

	MainMenu *_main_menu;
	
	struct Item {
		int id;
		std::string classname, animation;
		v2<int> position;
		int z;
		Uint32 dead_on;
		bool destroy_for_victory;
	};
	typedef std::vector<Item> Items;
	Items _items;
	std::vector<v2<int> > _specials;
	
	Alarm _check_items;
	void checkItems(const float dt);
	
	typedef std::map<const std::string, v2<int> > WaypointMap;
	typedef std::map<const std::string, WaypointMap> WaypointClassMap;
	typedef std::multimap<const std::string, std::string> WaypointEdgeMap;
	
	WaypointClassMap _waypoints;
	WaypointEdgeMap  _waypoint_edges;
	
	bool _show_fps, _show_log_lines;
	Object *_fps, *_log_lines;

	std::string _preload_map;
	bool _autojoin;

	float _shake;
	int _shake_int;
	
	Hud *_hud;
	bool _show_radar, _show_stats;
	int _loading_bar_total, _loading_bar_now;
	sdlx::Timer _timer;
	
	Credits *_credits;
	Cheater *_cheater;
	
	std::string _state;
	Alarm _state_timer;
	const sdlx::Font *_big_font;
	
	IGame(const IGame &);
	const IGame& operator=(const IGame &);
};

SINGLETON(Game, IGame);

#endif

