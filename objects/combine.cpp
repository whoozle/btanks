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

#include "alarm.h"
#include "resource_manager.h"
#include "config.h"
#include "object.h"
#include "item.h"
#include "game.h"

#include "math/unary.h"

class Combine : public Object {
public: 
	Combine();

	virtual void calculate(const float dt);
	virtual void tick(const float dt);
	virtual void onSpawn();
	virtual Object * clone() const;
	
	void emit(const std::string &event, Object * emitter);
	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_reaction_time);
		s.add(_waypoint_name);
		s.add(_stop);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_reaction_time);
		s.get(_waypoint_name);
		s.get(_stop);
	}	
private: 

	Alarm _reaction_time;
	bool _stop;
	std::string _waypoint_name;
};

void Combine::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
		spawn("corpse", "dead-" + animation, v2<float>::empty, v2<float>::empty);
	} 
	Object::emit(event, emitter);
}


void Combine::onSpawn() {
	GET_CONFIG_VALUE("objects.combine.reaction-time", float, rt, 0.5);
	_reaction_time.set(rt);

	play("hold", true);
	
	disown(); 
}

Combine::Combine() : Object("combine"), _reaction_time(true), _stop(false) //_refresh_waypoints(false) 
{}

void Combine::calculate(const float dt) {	
	if (!calculatingPath() && !isDriven()) {
		v2<float> waypoint;
		_velocity.clear();
		if (_waypoint_name.empty()) {
			_waypoint_name = getNearestWaypoint(registered_name + "s");
			assert(!_waypoint_name.empty());
			Game->getWaypoint(waypoint, registered_name + "s", _waypoint_name);
			//LOG_DEBUG(("%s[%d] moving to nearest waypoint at %g %g", animation.c_str(), getID(), waypoint.x, waypoint.y));
		} else {
			//LOG_DEBUG(("%s[%d] reached waypoint '%s'", animation.c_str(), getID(), _waypoint_name.c_str()));
			_waypoint_name = Game->getRandomWaypoint(registered_name + "s", _waypoint_name);
			Game->getWaypoint(waypoint, registered_name + "s", _waypoint_name);
			//LOG_DEBUG(("%s[%d] moving to next waypoint '%s' at %g %g", animation.c_str(), getID(), _waypoint_name.c_str(), waypoint.x, waypoint.y));
		}
		GET_CONFIG_VALUE("objects.combine.pathfinding-step", int, pfs, 16);
		findPath(waypoint.convert<int>(), pfs);
	}
	Way way;
	if (calculatingPath() && findPathDone(way)) {
		if (way.empty()) {
			LOG_DEBUG(("%s:%s[%d] no path. maybe commit a suicide?", registered_name.c_str(), animation.c_str(), getID()));
			//emit("death", NULL);
		}
		setWay(way);
	} else _velocity.clear();

	calculateWayVelocity();	
	
	GET_CONFIG_VALUE("objects.combine.rotation-time", float, rt, 0.1);
	limitRotation(dt, rt, true, false);
	updateStateFromVelocity();
}


void Combine::tick(const float dt) {
	Object::tick(dt);
	if (_velocity.is0() && getState() != "hold") {
		cancelAll();
		play("hold", true);
	} else if (!_velocity.is0() && getState() != "move") {
		cancelAll();
		play("move", true);
	}
}


Object * Combine::clone() const {
	return new Combine(*this);
}


REGISTER_OBJECT("combine", Combine, ());
