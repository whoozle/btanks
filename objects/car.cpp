
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
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		_refresh_waypoints.deserialize(s);
	}	
private: 
	Alarm _refresh_waypoints;
};

void Car::emit(const std::string &event, BaseObject * emitter) {
	if (event == "death") {
		spawn("corpse", "dead-" + animation, v3<float>::empty, v3<float>::empty);
	}
	Object::emit(event, emitter);
}


void Car::onSpawn() {
	GET_CONFIG_VALUE("objects.car.refreshing-path-interval", float, rpi, 1);
	_refresh_waypoints.set(rpi);
	play("hold", true);
}

Car::Car() : Object("car"), _refresh_waypoints(true) {}

void Car::calculate(const float dt) {	
	const bool refresh_path = _refresh_waypoints.tick(dt);
	if (!isDriven() && refresh_path) {
		LOG_DEBUG(("looking for waypoints..."));
		v3<int> waypoint;
		Game->getRandomWaypoint(waypoint, "cars");
		LOG_DEBUG(("next waypoint : %d %d", waypoint.x, waypoint.y));
		
		Way way;
		v3<float> w = waypoint.convert<float>();
		
		if (!findPath(w, way)) {
			LOG_WARN(("findPath failed. retry later."));
			_velocity.clear();
			return;
		}
		setWay(way);
	}
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
