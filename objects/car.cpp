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

class Car : public Object {
public: 
	Car();

	virtual void calculate(const float dt);
	virtual void tick(const float dt);
	virtual void onSpawn();
	virtual Object * clone() const;
	
	void emit(const std::string &event, BaseObject * emitter);
	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		_refresh_waypoints.serialize(s);
		_waypoint.serialize(s);
		_waypoint_rel.serialize(s);
		s.add(_waypoint_name);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		_refresh_waypoints.deserialize(s);
		_waypoint.deserialize(s);
		_waypoint_rel.deserialize(s);
		s.get(_waypoint_name);
	}	
private: 
	Alarm _refresh_waypoints;
	v3<float> _waypoint;
	v3<float> _waypoint_rel;
	std::string _waypoint_name;
};



void Car::emit(const std::string &event, BaseObject * emitter) {
	if (event == "death") {
		spawn("corpse", "dead-" + animation, v3<float>::empty, v3<float>::empty);
	} else if (event == "collision") {
		if (emitter != NULL) {
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
	GET_CONFIG_VALUE("objects.car.refreshing-path-interval", float, rpi, 1);
	_refresh_waypoints.set(rpi);
	play("hold", true);
}

Car::Car() : Object("car"), _refresh_waypoints(false) {}

void Car::calculate(const float dt) {	
	v3<float> position = getPosition();

	if (_waypoint_name.empty()) {
		_waypoint_name = getNearestWaypoint("car");
		assert(!_waypoint_name.empty());
		Game->getWaypoint(_waypoint, "car", _waypoint_name);
		_waypoint_rel = _waypoint - position;
		LOG_DEBUG(("moving to nearest waypoint at %g %g", _waypoint.x, _waypoint.y));
	}
	_velocity = _waypoint - position;

	if (_waypoint_rel.x != 0 && _velocity.x * _waypoint_rel.x <= 0)
		_velocity.x = 0;

	if (_waypoint_rel.y != 0 && _velocity.y * _waypoint_rel.y <= 0)
		_velocity.y = 0;

	if (_velocity.is0()) {
		_waypoint_name = Game->getRandomWaypoint("car", _waypoint_name);
		Game->getWaypoint(_waypoint, "car", _waypoint_name);
		_waypoint_rel = _waypoint - getPosition();
		LOG_DEBUG(("moving to next waypoint '%s' at %g %g", _waypoint_name.c_str(), _waypoint.x, _waypoint.y));
	}
	
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
