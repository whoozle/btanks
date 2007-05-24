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

#include "math/v2.h"
#include "player_state.h"
#include "alarm.h"

#include "export_btanks.h"
#include "sdlx/sdlx.h"

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

namespace sdlx {
	class Surface;
}

class BTANKSAPI IGame {

public: 
	DECLARE_SINGLETON(IGame);

	void init(const int argc, char *argv[]);
	void run();
	void deinit();
	
	void clear();
	void pause();

	IGame();
	~IGame();
	
	//stupid visual effect
	void shake(const float duration, const int intensity);
	
	void resetLoadingBar(const int total);
	void notifyLoadingBar(const int progress = 1);

	void loadMap(const std::string &name, const bool spawn = true, const bool skip_loadmap = false);	
	
	const std::string getRandomWaypoint(const std::string &classname, const std::string &last_wp = std::string()) const;
	const std::string getNearestWaypoint(const BaseObject *obj, const std::string &classname) const;
	void getWaypoint(v2<float> &wp, const std::string &classname, const std::string &name);
	
private:
	void onTick(const float dt);
	bool onKey(const SDL_keysym sym, const bool pressed);
	void onJoyButton(const int joy, const int id, const bool pressed);
	bool onMouse(const int button, const bool pressed, const int x, const int y);
	void onMenu(const std::string &name, const std::string &value);
	const std::string onConsole(const std::string &cmd, const std::string &param);

	void onEvent(const SDL_Event &event);
	void quit();
	
	void stopCredits();

	bool _paused, _map_loaded;

	MainMenu *_main_menu;
	
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
	
	Credits *_credits;
	Cheater *_cheater;
	
	const sdlx::Surface *_donate;
	float _donate_timer;
	
	IGame(const IGame &);
	const IGame& operator=(const IGame &);
};

SINGLETON(BTANKSAPI, Game, IGame);

#endif

