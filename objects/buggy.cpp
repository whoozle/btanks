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
#include "ai/waypoints.h"

class Buggy: public Object {
public: 
	Buggy(const std::string &classname) : Object(classname) {
		impassability = 1.0f;
	}
	virtual void calculate(const float dt);
	virtual void tick(const float dt);
	virtual void onSpawn();
	virtual Object * clone() const { return new Buggy(*this); }
	
	void emit(const std::string &event, Object * emitter);

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
	}

private: 
	virtual void getImpassabilityPenalty(const float impassability, float &base, float &base_value, float &penalty) const;
};

void Buggy::onSpawn() {
	play("hold", true);
	add("mod", "turrel", "buggy-gun", v2<float>(), Centered);
}

void Buggy::getImpassabilityPenalty(const float impassability, float &base, float &base_value, float &penalty) const {
	if (impassability >= 0.2) {
		base = 0.2;
		base_value = 0.6;
		penalty = 0;
		return;
	}
}

void Buggy::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
		if (registered_name == "static-buggy")
			detachVehicle();
		spawn("corpse", "dead-" + animation, v2<float>(), v2<float>());
	}
	Object::emit(event, emitter);
}

void Buggy::calculate(const float dt) {
	Object::calculate(dt);
	GET_CONFIG_VALUE("objects." + registered_name + ".rotation-time", float, rt, 0.05);
	limitRotation(dt, rt, true, false);
}

void Buggy::tick(const float dt) {
	Object::tick(dt);
	if (_velocity.is0() && getState() != "hold") {
		cancelAll();
		play("hold", true);
		get("mod")->emit("hold", this);
	} else if (!_velocity.is0() && getState() != "move") {
		cancelAll();
		play("move", true);
		get("mod")->emit("move", this);
	}
}

class AIBuggy : public Buggy, public ai::Waypoints {
public: 
	AIBuggy() : Buggy("fighting-vehicle") {}
	virtual void calculate(const float dt);
	virtual Object * clone() const {return new AIBuggy(*this);}
	virtual void onSpawn();

private:
	virtual void onObstacle(const Object *o);	
};

void AIBuggy::onSpawn() {
	Buggy::onSpawn();
	
	//obstacle_filter.insert("buggy");
	//obstacle_filter.insert("civilian");
	//obstacle_filter.insert("trooper");
	//obstacle_filter.insert("fighting-vehicle");
	_avoid_obstacles = true;

	ai::Waypoints::onSpawn(this);
	//GET_CONFIG_VALUE("objects.buggy.refreshing-path-interval", float, rpi, 1);
	//_refresh_waypoints.set(rpi);
}


void AIBuggy::onObstacle(const Object *o) {
/*
	if ((idx % 21) == 1) { //approx once per 5 second
		playRandomSound("klaxon", false);
	}
*/
}

void AIBuggy::calculate(const float dt) {
	ai::Waypoints::calculate(this, dt);

	float rt;
	Config->get("objects." + registered_name + ".rotation-time", rt, 0.05f);
	limitRotation(dt, rt, true, false);
	updateStateFromVelocity();
}

REGISTER_OBJECT("static-buggy", Buggy, ("fighting-vehicle"));
REGISTER_OBJECT("buggy", AIBuggy, ());
