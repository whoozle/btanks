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
#include "ai/waypoints.h"

#include "math/unary.h"

class Combine : public Object, public ai::Waypoints {
public: 
	Combine();

	virtual void calculate(const float dt);
	virtual void tick(const float dt);
	virtual void onSpawn();
	virtual Object * clone() const;
	
	void emit(const std::string &event, Object * emitter);
	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
	}	
	
	virtual void onObstacle(const int idx);

};

void Combine::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
		spawn("corpse", "dead-" + animation, v2<float>::empty, v2<float>::empty);
	} 
	Object::emit(event, emitter);
}

void Combine::onObstacle(const int idx) {
	if ((idx % 21) == 1) { //approx once per 5 second
		playRandomSound("klaxon", false);
	}
}


void Combine::onSpawn() {
	_avoid_obstacles = true;
	ai::Waypoints::onSpawn(this);
	
	play("hold", true);
	
	disown(); 
}

Combine::Combine() : Object("combine") {}

void Combine::calculate(const float dt) {	
	ai::Waypoints::calculate(this, dt);
	
	float rt;
	Config->get("objects." + registered_name + ".rotation-time", rt, 0.1f);
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
