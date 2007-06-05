#ifndef __BTANKS_PLAYER_SLOT_H__
#define __BTANKS_PLAYER_SLOT_H__

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

#include <string>
#include <set>
#include <queue>

#include "mrt/serializable.h"
#include "math/v2.h"
#include "math/v3.h"
#include "sdlx/rect.h"
#include "player_state.h"

class Object;
class ControlMethod;
class Tooltip;

class PlayerSlot : public mrt::Serializable {
public:
	int id;
	ControlMethod * control_method;
	PlayerState old_state; //help broadcast AI state changes.
	
	v3<int> position;
		
	bool need_sync, dont_interpolate;
	bool remote;
	float trip_time;
	
	bool visible;
	sdlx::Rect viewport;
	
	v2<float> map_pos, map_vel, map_dst, map_dst_vel, map_dst_pos;
	v2<int> map_dpos;
		
	//respawn stuff.
	std::string classname;
	std::string animation;
	
	int frags;
	bool reserved;
	
	std::set<int> zones_reached;
	int spawn_limit;
	
	int score;
	
	PlayerSlot();
	PlayerSlot(const int id);
	void clear();
	
	Object * getObject(); 
	const Object * getObject() const;
	~PlayerSlot();
		
	typedef std::queue<std::pair<float, Tooltip *> > Tooltips;
	Tooltips tooltips;
	
	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);
	
	void displayLast();
	void tick(const float dt);
	
	void createControlMethod(const std::string &name);
	void spawnPlayer(const std::string &classname, const std::string &animation);
	void validatePosition(v2<float>& position);
	
private: 
	Tooltip * last_tooltip;
};

#endif
