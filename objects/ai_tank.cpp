
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

#include "alarm.h"
#include "tank.h"
#include "tmx/map.h"
#include "sdlx/rect.h"
#include "mrt/logger.h"
#include "resource_manager.h"
#include "config.h"

class AITank : public Tank {
public: 
	AITank();
	AITank(const std::string &animation);

	virtual void calculate(const float dt);
	virtual void onSpawn();
	virtual Object * clone() const;
	virtual void serialize(mrt::Serializator &s) const {
		Tank::serialize(s);
		_reaction_time.serialize(s);
		_refresh_waypoints.serialize(s);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Tank::deserialize(s);
		_reaction_time.deserialize(s);
		_refresh_waypoints.deserialize(s);
	}	
private: 
	Alarm _reaction_time, _refresh_waypoints;
};


void AITank::onSpawn() {
	GET_CONFIG_VALUE("objects.ai-tank.reaction-time", float, reaction_time, 0.1);
	_reaction_time.set(reaction_time);
	GET_CONFIG_VALUE("objects.ai-tank.refreshing-path-interval", float, rpi, 0.3);
	_refresh_waypoints.set(rpi);
	Tank::onSpawn();
}

AITank::AITank() : Tank(), 
	_reaction_time(true), _refresh_waypoints(true) {}

AITank::AITank(const std::string &animation) : Tank(animation), 
	_reaction_time(true), _refresh_waypoints(true)  {}

void AITank::calculate(const float dt) {	
	//LOG_DEBUG(("dt = %f", dt));
	_state.fire = false;
	calculateWayVelocity();
	
	if (!_reaction_time.tick(dt)) {
		return;
	}
	
	v3<float> bpos, pos, vel;
	bool found_bullet = false;

	if (getNearest("bullet", bpos, vel)) {
		//LOG_DEBUG(("AAA!!!"));
		float t = getCollisionTime(bpos, vel, (size.x + size.y)/2);
		//LOG_DEBUG(("collision time: %f", t));
		if (t >= 0) {
			_velocity.x = -vel.y;
			_velocity.y = vel.x;
			found_bullet = true;
		}
	} 
	_velocity.quantize8();

	Way way;
	const bool refresh_path = _refresh_waypoints.tick(dt);

	GET_CONFIG_VALUE("objects.ai-tank.dead-zone-for-target", float, deadzone, 2.5);
	
	if (getNearest("player", pos, vel, (refresh_path || !isDriven())?&way:0)) {
		//LOG_DEBUG(("found human: %f %f", pos.x, pos.y));
		const bool player_close = pos.length() < IMap::pathfinding_step * deadzone;
		
		if (found_bullet && bpos.quick_length() < pos.quick_length()) {
			//LOG_DEBUG(("bpos: %g, player: %g", bpos.quick_length(), pos.quick_length()));
			return;
		}
		
		if (player_close) {
			if (isDriven())
				LOG_DEBUG(("player is too close, turning off waypoints..."));
			way.clear();
			setWay(way);
		}
		
		if (!way.empty()) {
			//LOG_DEBUG(("finding path..."));
			way.pop_back();
			setWay(way);
		} else {	
			if (!isDriven()) {
				_velocity = pos; //straight to player.
				_velocity.quantize8();
			}
		} 

		static float threshold = 12;
		
		if (pos.x >= -threshold && pos.x <= threshold) { 
			pos.x = 0;
			_state.fire = true;
		}
		if (pos.y >= -threshold && pos.y <= threshold) {
			pos.y = 0;
			_state.fire = true;
		}

		float tg = pos.x != 0 ?(pos.y / pos.x - 1):100;
		if (tg < 0) tg = -tg;
		
		//LOG_DEBUG(("tg = %f", tg));
		if (tg > 0.577350269189625798 && tg < 1.7320508075688778) {
			_state.fire = true;
		}
		
		if (pos.length() < 3*IMap::pathfinding_step / 2) {
			_velocity.clear();
			return;
		}

		//LOG_DEBUG(("v: %f %f", pos.x, pos.y));
	  } else {
	  	_velocity.clear();
	  }
//	LOG_DEBUG(("v: %g %g", _velocity.x, _velocity.y));
	GET_CONFIG_VALUE("objects.tank.rotation-time", float, rt, 0.05);
	limitRotation(dt, 8, rt, true, false);
	updateStateFromVelocity();
}

Object * AITank::clone() const {
	return new AITank(*this);
}


REGISTER_OBJECT("ai-tank", AITank, ());
