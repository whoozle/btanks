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
#include "world.h"
#include "mrt/random.h"
#include "math/unary.h"

class Car : public Object {
public: 
	Car();

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
		s.add(_obstacle);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_reaction_time);
		s.get(_waypoint_name);
		s.get(_stop);
		s.get(_obstacle);
	}	
private: 
	virtual void getImpassabilityPenalty(const float impassability, float &base, float &base_value, float &penalty) const;

	Alarm _reaction_time;
	bool _stop;
	std::string _waypoint_name;
	int _obstacle;
};

void Car::getImpassabilityPenalty(const float impassability, float &base, float &base_value, float &penalty) const {
	if (impassability >= 0.2) {
		base = 0.2;
		base_value = 0.8;
		return;
	}
}
/*
const int Car::getPenalty(const int map_im, const int obj_im) const {
	return (map_im >= 20 || obj_im >= 20)?5000:0;
}
*/

void Car::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
		spawn("corpse", "dead-" + animation, v2<float>::empty, v2<float>::empty);
	} else if (event == "collision") {
		if (emitter != NULL && emitter->speed > 0) {
			Item * item = dynamic_cast<Item *>(emitter);
			//no items.
			if (item == NULL) {
				GET_CONFIG_VALUE("objects.car.damage", int, d, 5);
				emitter->addDamage(this, d);
				emitter->addEffect("stunned", 0.1f);
				emit("death", emitter);
			}
		}
	}
	Object::emit(event, emitter);
}


void Car::onSpawn() {
	GET_CONFIG_VALUE("objects.car.reaction-time", float, rt, 0.1);
	_reaction_time.set(rt);
	mrt::randomize(rt, rt / 10);
	//GET_CONFIG_VALUE("objects.car.refreshing-path-interval", float, rpi, 1);
	//_refresh_waypoints.set(rpi);
	play("hold", true);
	
	disown(); 
}

Car::Car() : Object("car"), _reaction_time(true), _stop(false), _obstacle(0)
{}

void Car::calculate(const float dt) {
	if (_reaction_time.tick(dt)) {
		static std::set<std::string> filter;
		if (filter.empty()) {
			filter.insert("car");
			filter.insert("civilian");
			filter.insert("trooper");
			filter.insert("player");
		}
	
		std::set<const Object *> objs;
		World->enumerateObjects(objs, this, (size.x + size.y) * 2 / 3, &filter);
		std::set<const Object *>::const_iterator i;
		for(i = objs.begin(); i != objs.end(); ++i) {
			if ((*i)->speed == 0)
				continue;
			
			v2<float> dpos = getRelativePosition(*i);
			if (dpos.same_sign(_direction)) {
				if (_obstacle == 0)
					_obstacle = 1; //keep obstacle value incrementing
				_velocity.clear();
				break;
			}
		}
		if (i == objs.end())
			_obstacle = 0;
		
		if (_obstacle) {
			if ((_obstacle % 21) == 1) { //approx once per 5 second
				playRandomSound("klaxon", false);
			}
			++_obstacle;
		}
	}
	
	if (_obstacle) {
		_velocity.clear();
		return;
	}
	
	if (!calculatingPath() && !isDriven()) {
		v2<float> waypoint;
		_velocity.clear();
		if (_waypoint_name.empty()) {
			_waypoint_name = getNearestWaypoint("cars");
			assert(!_waypoint_name.empty());
			Game->getWaypoint(waypoint, "cars", _waypoint_name);
			//LOG_DEBUG(("%s[%d] moving to nearest waypoint at %g %g", animation.c_str(), getID(), waypoint.x, waypoint.y));
		} else {
			//LOG_DEBUG(("%s[%d] reached waypoint '%s'", animation.c_str(), getID(), _waypoint_name.c_str()));
			_waypoint_name = Game->getRandomWaypoint("cars", _waypoint_name);
			Game->getWaypoint(waypoint, "cars", _waypoint_name);
			//LOG_DEBUG(("%s[%d] moving to next waypoint '%s' at %g %g", animation.c_str(), getID(), _waypoint_name.c_str(), waypoint.x, waypoint.y));
		}
		GET_CONFIG_VALUE("objects.car.pathfinding-step", int, pfs, 16);
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
	
	GET_CONFIG_VALUE("objects.car.rotation-time", float, rt, 0.05);
	limitRotation(dt, rt, true, false);
	updateStateFromVelocity();
}


void Car::tick(const float dt) {
	Object::tick(dt);
	if (_velocity.is0() && getState() != "hold") {
		cancelAll();
		play("hold", true);
	} else if (!_velocity.is0() && getState() != "move") {
		cancelAll();
		play("move", true);
	}
}


Object * Car::clone() const {
	return new Car(*this);
}


REGISTER_OBJECT("car", Car, ());
