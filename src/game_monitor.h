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

#include <vector>
#include <string>
#include <set>

#include "mrt/singleton.h"
#include "mrt/export.h"
#include "alarm.h"
#include "math/v2.h"
#include "sdlx/sdlx.h"

namespace sdlx {
class Surface;
}

struct Item {
	Item(const std::string &classname, const std::string &animation, const v2<int> position, const int z = 0) :
		classname(classname), animation(animation), position(position), z(z), id(-1), dead_on(0), 
		destroy_for_victory(false)
		{}

	std::string classname, animation;
	v2<int> position;
	int z;

	int id;
	Uint32 dead_on;
	bool destroy_for_victory;
	std::string save_for_victory;
};


class DLLEXPORT IGameMonitor {
public:
	IGameMonitor();
	DECLARE_SINGLETON(IGameMonitor);

	void add(const Item &item);	
	void checkItems(const float dt);
	
	const std::vector<v2<int> >& getSpecials() const { return _specials; }
	const size_t getItemsCount() const { return _items.size(); }

	void gameOver(const std::string &area, const std::string &message, const float time);
	void displayMessage(const std::string &area, const std::string &message, const float time);
	void setTimer(const std::string &area, const std::string &message, const float time);
	void resetTimer();
	const float getTimer() const { return _timer; }

	void clear();
	
	void tick(const float dt);

	void pushState(const std::string &state, const float time);
	const std::string popState(const float dt);
	
	void render(sdlx::Surface &window);
	
	const bool disabled(const std::string &classname) const;
	void disable(const std::string &classname, const bool value = true);

private:

	bool _game_over;

	typedef std::vector<Item> Items;
	Items _items;
	std::vector<v2<int> > _specials;

	Alarm _check_items;

	//displaying messages	
	std::string _state;
	Alarm _state_timer;
	
	std::string _timer_message, _timer_message_area;
	float _timer;
	
	std::set<std::string> _disabled;
};

SINGLETON(GameMonitor, IGameMonitor);

#endif

