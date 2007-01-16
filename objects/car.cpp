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
#include "resource_manager.h"
#include "config.h"
#include "object.h"
#include "game.h"
#include "item.h"

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
		_reaction_time.serialize(s);
		//_refresh_waypoints.serialize(s);
		//_waypoint.serialize(s);
		//_waypoint_rel.serialize(s);
		s.add(_waypoint_name);
		s.add(_stop);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		_reaction_time.deserialize(s);
		//_refresh_waypoints.deserialize(s);
		//_waypoint.deserialize(s);
		//_waypoint_rel.deserialize(s);
		s.get(_waypoint_name);
		s.get(_stop);
	}	
private: 
	virtual const int getPenalty(const int map_im, const int obj_im) const;

	Alarm _reaction_time;
	bool _stop;
	//Alarm _refresh_waypoints;
	//v3<float> _waypoint;
	//v3<float> _waypoint_rel;
	std::string _waypoint_name;
};

const int Car::getPenalty(const int map_im, const int obj_im) const {
	return(map_im >= 20 || obj_im >= 20)?5000:0;
}



void Car::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
		spawn("corpse", "dead-" + animation, v3<float>::empty, v3<float>::empty);
	} else if (event == "collision") {
		if (emitter != NULL && emitter->speed > 0) {
			Item * item = dynamic_cast<Item *>(emitter);
			//no items.
			if (item == NULL) {
				GET_CONFIG_VALUE("objects.car.damage", int, d, 5);
				emitter->addDamage(this, d);
				emit("death", emitter);
			}
		}
	}
	Object::emit(event, emitter);
}


void Car::onSpawn() {
	GET_CONFIG_VALUE("objects.car.reaction-time", float, rt, 0.1);
	_reaction_time.set(rt);
	//GET_CONFIG_VALUE("objects.car.refreshing-path-interval", float, rpi, 1);
	//_refresh_waypoints.set(rpi);
	play("hold", true);
	
	disown(); 
}

Car::Car() : Object("car"), _reaction_time(true), _stop(false) //_refresh_waypoints(false) 
{}

/*
void Car::calculate(const float dt) {	
	v3<float> position = getPosition();

	if (_waypoint_name.empty()) {
		_waypoint_name = getNearestWaypoint("cars");
		assert(!_waypoint_name.empty());
		Game->getWaypoint(_waypoint, "cars", _waypoint_name);
		_waypoint_rel = _waypoint - position;
		LOG_DEBUG(("%s[%d] moving to nearest waypoint at %g %g", animation.c_str(), getID(), _waypoint.x, _waypoint.y));
	}
	_velocity = _waypoint - position;
	GET_CONFIG_VALUE("engine.allowed-pathfinding-fault", int, f, 5);

	if ((_waypoint_rel.x != 0 && _velocity.x * _waypoint_rel.x <= 0) || (math::abs(_velocity.x) < f))
		_velocity.x = 0;

	if ((_waypoint_rel.y != 0 && _velocity.y * _waypoint_rel.y <= 0) || (math::abs(_velocity.y) < f))
		_velocity.y = 0;

	if (_velocity.is0()) {
		LOG_DEBUG(("%s[%d] reached waypoint '%s'", animation.c_str(), getID(), _waypoint_name.c_str()));
		_waypoint_name = Game->getRandomWaypoint("cars", _waypoint_name);
		Game->getWaypoint(_waypoint, "cars", _waypoint_name);
		_waypoint_rel = _waypoint - getPosition();
		LOG_DEBUG(("%s[%d] moving to next waypoint '%s' at %g %g", animation.c_str(), getID(), _waypoint_name.c_str(), _waypoint.x, _waypoint.y));
	}
	
	GET_CONFIG_VALUE("objects.car.rotation-time", float, rt, 0.05);
	limitRotation(dt, rt, true, false);
	updateStateFromVelocity();
}
*/

void Car::calculate(const float dt) {	
	if (!calculatingPath() && !isDriven()) {
		v3<float> waypoint;
		_velocity.clear();
		if (_waypoint_name.empty()) {
			_waypoint_name = getNearestWaypoint("cars");
			assert(!_waypoint_name.empty());
			Game->getWaypoint(waypoint, "cars", _waypoint_name);
			LOG_DEBUG(("%s[%d] moving to nearest waypoint at %g %g", animation.c_str(), getID(), waypoint.x, waypoint.y));
		} else {
			LOG_DEBUG(("%s[%d] reached waypoint '%s'", animation.c_str(), getID(), _waypoint_name.c_str()));
			_waypoint_name = Game->getRandomWaypoint("cars", _waypoint_name);
			Game->getWaypoint(waypoint, "cars", _waypoint_name);
			LOG_DEBUG(("%s[%d] moving to next waypoint '%s' at %g %g", animation.c_str(), getID(), _waypoint_name.c_str(), waypoint.x, waypoint.y));
		}
		GET_CONFIG_VALUE("object.car.pathfinding-step", int, pfs, 16);
		findPath(waypoint.convert<int>(), pfs);
		//_velocity = waypoint - getPosition();
	}
	Way way;
	if (calculatingPath() && findPathDone(way)) {
		setWay(way);
	}

	calculateWayVelocity();	
	if (!isDriven())
		_velocity.clear();
	/*
	if (_reaction_time.tick(dt)) {
		v3<float> pos, vel;
		GET_CONFIG_VALUE("objects.car.stop-distance", int, sd, 32);
		_stop = false;
		const Object *tl = getNearestObject("traffic-lights");
		if (tl) {
			v3<float> rel = getRelativePosition(tl);
			if (rel.quick_length() < sd * sd) {
				const std::string tl_state = tl->getState();
				LOG_DEBUG(("%s[%d] traffic light [%s]", animation.c_str(), getID(), tl_state.c_str()));
				const bool red = (tl_state == "flashing-red" || tl_state == "red" || tl_state == "yellow");
				const bool green = (tl_state == "flashing-green" || tl_state == "green" || tl_state == "yellow");
				if ( (red && _velocity.y != 0) || (green && _velocity.x != 0) ) {
					LOG_DEBUG(("stop!"));
					_stop = true;
				}
			}
		}
	}
	
	if (_stop)
		_velocity.clear();
	*/
	
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
