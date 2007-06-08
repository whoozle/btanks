#ifndef BTANKS_GAME_MONITOR_H__
#define BTANKS_GAME_MONITOR_H__

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

#include <deque>
#include <string>
#include <set>
#include <map>

#include "mrt/singleton.h"
#include "mrt/serializable.h"
#include "alarm.h"
#include "math/v2.h"
#include "math/v3.h"
#include "sdlx/sdlx.h"
#include "export_btanks.h"

namespace sdlx {
class Surface;
class Rect;
}

class BaseObject;
class Object;

struct BTANKSAPI GameItem {
	GameItem(const std::string &classname, const std::string &animation, const std::string &property, const v2<int> position, const int z = 0) :
		classname(classname), animation(animation), property(property), position(position), z(z), id(-1), dead_on(0), 
		destroy_for_victory(false)
		{}
	void respawn();
	void updateMapProperty();

	std::string classname, animation, property;
	v2<int> position;
	int z;

	int id;
	Uint32 dead_on;
	bool destroy_for_victory;
	std::string save_for_victory;
};


class BTANKSAPI IGameMonitor {
public:
	IGameMonitor();
	DECLARE_SINGLETON(IGameMonitor);

	void add(const GameItem &item);	
	GameItem& find(const std::string &property);
	void eraseLast(const std::string &property);
	GameItem& find(const Object *o);
	const GameItem& find(const Object *o) const;
	const std::string generatePropertyName(const std::string &prefix);
	
	void checkItems(const float dt);
	
	const std::vector<v3<int> >& getSpecials() const { return _specials; }
	const size_t getItemsCount() const { return _items.size(); }

	void gameOver(const std::string &area, const std::string &message, const float time, const bool win);
	void displayMessage(const std::string &area, const std::string &message, const float time);
	void setTimer(const std::string &area, const std::string &message, const float time, const bool win_at_end);
	void resetTimer();

	void clear();
	
	void tick(const float dt);

	void pushState(const std::string &state, const float time);
	const std::string popState(const float dt);
	
	void render(sdlx::Surface &window);
	
	const bool disabled(const Object *o) const;
	void disable(const std::string &classname, const bool value = true);

	void serialize(mrt::Serializator &s) const;
	void deserialize(const mrt::Serializator &s);
	
	void killAllClasses(const std::set<std::string> &classes);

	void loadMap(const std::string &campaign, const std::string &name, const bool spawn = true, const bool skip_loadmap = false);		

	//waypoints
	const std::string getRandomWaypoint(const std::string &classname, const std::string &last_wp = std::string()) const;
	const std::string getNearestWaypoint(const BaseObject *obj, const std::string &classname) const;
	void getWaypoint(v2<float> &wp, const std::string &classname, const std::string &name);
	
	void renderWaypoints(sdlx::Surface &surface, const sdlx::Rect &src, const sdlx::Rect &viewport);	

private:

	bool _game_over, _win;

	typedef std::deque<GameItem> Items;
	Items _items;
	std::vector<v3<int> > _specials;

	Alarm _check_items;

	//displaying messages	
	std::string _state;
	Alarm _state_timer;
	
	std::string _timer_message, _timer_message_area;
	float _timer;
	bool _timer_win_at_end;
	
	std::set<std::string> _disabled;
	std::set<std::string> _destroy_classes;
	
	//waypoints stuff
	typedef std::map<const std::string, v2<int> > WaypointMap;
	typedef std::map<const std::string, WaypointMap> WaypointClassMap;
	typedef std::multimap<const std::string, std::string> WaypointEdgeMap;
	
	WaypointMap 	 _all_waypoints;
	WaypointClassMap _waypoints;
	WaypointEdgeMap  _waypoint_edges;	
	
	std::string _campaign;
};

SINGLETON(BTANKSAPI, GameMonitor, IGameMonitor);

#endif

