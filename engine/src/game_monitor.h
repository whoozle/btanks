#ifndef BTANKS_GAME_MONITOR_H__
#define BTANKS_GAME_MONITOR_H__

/* Battle Tanks Game
 * Copyright (C) 2006-2009 Battle Tanks team
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
#include "menu/box.h"
#include "sl08/sl08.h"
#include "team.h"

namespace sdlx {
class Surface;
class Rect;
}

class BaseObject;
class Object;
class Campaign;
class PlayerSlot;
class SpecialZone;

struct BTANKSAPI GameItem {
	GameItem(const std::string &classname, const std::string &animation, const std::string &property, const v2<int> position, const int z = 0) :
		classname(classname), animation(animation), property(property), position(position), z(z), dir(0), id(-1), spawn_limit(-1), dead_on(0), 
		destroy_for_victory(false), hidden(false), special(false) {}
	void respawn();
	void kill();
	void renameProperty(const std::string &name);
	void updateMapProperty();
	void setup(const std::string &name, const std::string &subname);

	std::string classname, animation, property;
	v2<int> position;
	int z, dir;

	int id, spawn_limit;
	Uint32 dead_on;
	bool destroy_for_victory;
	std::string save_for_victory;
	bool hidden, special;
};

class LuaHooks;

class BTANKSAPI IGameMonitor {
public:
	IGameMonitor();
	~IGameMonitor();
	DECLARE_SINGLETON(IGameMonitor);

	void add(const GameItem &item, bool dont_respawn = false);	
	GameItem& find(const std::string &property);
	void eraseLast(const std::string &property);
	GameItem& find(const Object *o);
	const GameItem& find(const Object *o) const;
	const std::string generatePropertyName(const std::string &prefix);
	
	void checkItems(const float dt);
	
	const std::vector<v3<int> >& getSpecials() const { return _specials; }
	const std::vector<v3<int> >& getFlags() const { return _flags; }
	
	const size_t getItemsCount() const { return _items.size(); }

	void game_over(const std::string &area, const std::string &message, float time, const bool win);
	void displayMessage(const std::string &area, const std::string &message, float time, const bool global = false);
	void hideMessage();
	void setTimer(const std::string &area, const std::string &message, float time, const bool win_at_end);
	void resetTimer();

	void clear();
	
	void tick(const float dt);

	void pushState(const std::string &state, float time);
	const std::string popState(const float dt);
	
	void render(sdlx::Surface &window);
	
	const bool disabled(const Object *o) const;
	void disable(const std::string &classname, const bool value = true);

	void serialize(mrt::Serializator &s) const;
	void deserialize(const mrt::Serializator &s);
	
	void killAllClasses(const std::set<std::string> &classes);

	void loadMap(Campaign * campaign, const std::string &name, const bool spawn = true, const bool skip_loadmap = false);
	void startGame(Campaign *campaign, const std::string &name);

	//waypoints
	const bool hasWaypoints(const std::string &classname) const;
	const std::string getRandomWaypoint(const std::string &classname, const std::string &last_wp = std::string()) const;
	const std::string get_nearest_waypoint(const Object *obj, const std::string &classname) const;
	void get_waypoint(v2<float> &wp, const std::string &classname, const std::string &name);
	
	void renderWaypoints(sdlx::Surface &surface, const sdlx::Rect &src, const sdlx::Rect &viewport);	

	void addBonuses(const PlayerSlot &slot);
	
	const Campaign * getCampaign() const {return _campaign; }
	
	const bool game_over() const { return _game_over; }

	void onScriptZone(const int slot_id, const SpecialZone &zone, const bool global);
	void setSpecials(const std::vector<int> &ex) { _external_specials = ex; }
	
	const bool usedInCampaign(const std::string &base, const std::string &id) const;
	const void useInCampaign(const std::string &base, const std::string &id);

	sl08::slot4<void, int, int, int, int, IGameMonitor> on_map_resize_slot;	
	void parseWaypoints(int, int, int, int);
	
	void onTooltip(const std::string &event, const int slot_id, const std::string &area, const std::string &message);
	
	void startGameTimer(const std::string &name, const float period, const bool repeat);
	void stopGameTimer(const std::string &name);
	
	const int getBase(const Team::ID id) const;

private:
	sl08::slot1<void, const Object *, IGameMonitor> add_object_slot;
	sl08::slot1<void, const Object *, IGameMonitor> delete_object_slot;
	void addObject(const Object *o);
	void deleteObject(const Object *o);

	void saveCampaign();

	sl08::slot2<const std::string, const std::string &, const std::string &, IGameMonitor> on_console_slot;
	const std::string onConsole(const std::string &cmd, const std::string &param);

	bool _game_over, _win;

	typedef std::deque<GameItem> Items;
	Items _items;
	std::vector<int> _flag_id;
	std::vector<v3<int> > _specials, _flags;
	std::vector<int> _external_specials;

	Alarm _check_items;

	//displaying messages	
	Box _state_bg;
	std::string _state;
	Alarm _state_timer;
	
	std::string _timer_message, _timer_message_area;
	float _timer;
	bool _timer_win_at_end;
	
	std::set<std::string> _disabled;
	std::set<std::string> _destroy_classes;
	std::set<int> _present_objects;
	bool _objects_limit_reached;
	
	//waypoints stuff
	typedef std::map<const std::string, v2<int> > WaypointMap;
	typedef std::map<const std::string, WaypointMap> WaypointClassMap;
	typedef std::multimap<const std::string, std::string> WaypointEdgeMap;
	
	WaypointMap 	 _all_waypoints;
	WaypointClassMap _waypoints;
	WaypointEdgeMap  _waypoint_edges;	
	
	Campaign * _campaign;
	
	struct GameBonus {
		std::string classname, animation;
		int id;
		GameBonus(const std::string &classname, const std::string &animation, const int id) : 
			classname(classname), animation(animation), id(id) {}
	};
	std::vector<GameBonus> bonuses;
#ifdef ENABLE_LUA
	LuaHooks* lua_hooks;
#endif

	std::set<std::pair<std::string, std::string> > used_maps;
	
	void processGameTimers(const float dt);
	struct Timer {
		float t, period;
		bool repeat;
		Timer(const float period, const bool repeat): t(0), period(period), repeat(repeat) {}
	};
	typedef std::map<const std::string, Timer> Timers;
	Timers timers;
	
	int team_base[4];
	float total_time;
};

PUBLIC_SINGLETON(BTANKSAPI, GameMonitor, IGameMonitor);

#endif

