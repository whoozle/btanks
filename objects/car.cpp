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

#include "resource_manager.h"
#include "config.h"
#include "object.h"
#include "item.h"
#include "ai/waypoints.h"

class Car: public Object {
public: 
	Car(const std::string &classname) : Object(classname), _alt_fire(1, false) {}
	virtual void calculate(const float dt);
	virtual void tick(const float dt);
	virtual void onSpawn();
	virtual Object * clone() const { return new Car(*this); }
	
	void emit(const std::string &event, Object * emitter);

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_alt_fire);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_alt_fire);
	}


protected:
	Alarm _alt_fire;
private: 
	virtual void getImpassabilityPenalty(const float impassability, float &base, float &base_value, float &penalty) const;
};

void Car::onSpawn() {
	play("hold", true);
}

void Car::getImpassabilityPenalty(const float impassability, float &base, float &base_value, float &penalty) const {
	if (impassability >= 0.2) {
		base = 0.2;
		base_value = 0.6;
		penalty = 0;
		return;
	}
}

void Car::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
		if (registered_name == "static-car")
			detachVehicle();
		spawn("corpse", "dead-" + animation, v2<float>(), v2<float>());
	} else if (event == "collision" && !_variants.has("safe")) {
		if (emitter != NULL && emitter->speed > 0) {
			if (emitter->registered_name == "machinegunner-player" && registered_name.compare(0, 7, "static-") == 0) {
				return;
			}
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

void Car::calculate(const float dt) {
	Object::calculate(dt);
	GET_CONFIG_VALUE("objects." + registered_name + ".rotation-time", float, rt, 0.05);
	limitRotation(dt, rt, true, false);
}

void Car::tick(const float dt) {
	if (_alt_fire.tick(dt) && _state.alt_fire) {
		_alt_fire.reset();
		playRandomSound("klaxon", false);
	}
	Object::tick(dt);
	if (_velocity.is0() && getState() != "hold") {
		cancelAll();
		play("hold", true);
	} else if (!_velocity.is0() && getState() != "move") {
		cancelAll();
		play("move", true);
	}
}

class AICar : public Car, public ai::Waypoints {
public: 
	AICar() : Car("car"){}
	virtual void calculate(const float dt);
	virtual Object * clone() const {return new AICar(*this);}
	virtual void onSpawn();

private:
	virtual void onObstacle(const Object *o);	
};

void AICar::onSpawn() {
	Car::onSpawn();
	//obstacle_filter.insert("car");
	//obstacle_filter.insert("civilian");
	//obstacle_filter.insert("trooper");
	//obstacle_filter.insert("player");
	_avoid_obstacles = true;

	ai::Waypoints::onSpawn(this);
	_alt_fire.set(5);
	//GET_CONFIG_VALUE("objects.car.refreshing-path-interval", float, rpi, 1);
	//_refresh_waypoints.set(rpi);
	
	disown(); 
}


void AICar::onObstacle(const Object *o) {
/*
	if ((idx % 21) == 1) { //approx once per 5 second
		playRandomSound("klaxon", false);
	}
*/
	_state.alt_fire = true;
}

void AICar::calculate(const float dt) {
	ai::Waypoints::calculate(this, dt);

	float rt;
	Config->get("objects." + registered_name + ".rotation-time", rt, 0.05f);
	limitRotation(dt, rt, true, false);
	updateStateFromVelocity();
}

REGISTER_OBJECT("static-car", Car, ("vehicle"));
REGISTER_OBJECT("car", AICar, ());
