/* Battle Tanks Game
 * Copyright (C) 2006-2009 Battle Tanks team
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

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
*/

#include "registrar.h"
#include "config.h"
#include "object.h"
#include "ai/waypoints.h"
#include "special_owners.h"

class Buggy: public Object {
public: 
	Buggy(const std::string &classname) : Object(classname) {
		impassability = 1.0f;
	}
	virtual void calculate(const float dt);
	virtual void tick(const float dt);
	virtual void on_spawn();
	virtual Object * clone() const { return new Buggy(*this); }
	
	void emit(const std::string &event, Object * emitter);

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
	}

private: 
	virtual void get_impassability_penalty(const float impassability, float &base, float &base_value, float &penalty) const;
};

void Buggy::on_spawn() {
	if (registered_name.substr(0, 6) == "static") {
		remove_owner(OWNER_MAP);
		disable_ai = true;
	}

	play("hold", true);
	bool ai = registered_name == "buggy" && has_owner(OWNER_MAP);
	Object *turrel = add("mod", ai?"turrel-on-buggy(ground-aim)":"turrel-on-buggy", "buggy-gun", v2<float>(), Centered);
	turrel->set_z(get_z() + 5, true);

}

void Buggy::get_impassability_penalty(const float impassability, float &base, float &base_value, float &penalty) const {
	if (impassability >= 0.2) {
		base = 0.2;
		base_value = 0.6;
		penalty = 0;
		return;
	}
}

void Buggy::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
		spawn("corpse", "dead-" + animation, v2<float>(), v2<float>());
		_dead = true;
		detachVehicle();
		Object::emit(event, emitter);
	} else {
		Object::emit(event, emitter);
	}
}

void Buggy::calculate(const float dt) {
	Object::calculate(dt);
	GET_CONFIG_VALUE("objects." + registered_name + ".rotation-time", float, rt, 0.05);
	limit_rotation(dt, rt, true, false);
}

void Buggy::tick(const float dt) {
	Object::tick(dt);

	if (!playing_sound("vehicle-sound")) {
		play_sound("vehicle-sound", true, 0.4f);
	}

	if (_velocity.is0() && get_state() != "hold") {
		cancel_all();
		play("hold", true);
		get("mod")->emit("hold", this);
	} else if (!_velocity.is0() && get_state() != "move") {
		cancel_all();
		play("move", true);
		get("mod")->emit("move", this);
	}
}

class AIBuggy : public Buggy, public ai::Waypoints {
public: 
	AIBuggy(const std::string &classname) : Buggy(classname) {}
	virtual void calculate(const float dt);
	virtual Object * clone() const {return new AIBuggy(*this);}
	virtual void on_spawn();
	virtual void serialize(mrt::Serializator &s) const {
		Buggy::serialize(s);
		ai::Waypoints::serialize(s);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Buggy::deserialize(s);
		ai::Waypoints::deserialize(s);
	}

private:
	virtual void onObstacle(const Object *o);	
};

void AIBuggy::on_spawn() {
	Buggy::on_spawn();
	
	//obstacle_filter.insert("buggy");
	//obstacle_filter.insert("civilian");
	//obstacle_filter.insert("trooper");
	//obstacle_filter.insert("fighting-vehicle");
	_avoid_obstacles = true;

	ai::Waypoints::on_spawn(this);
	//GET_CONFIG_VALUE("objects.buggy.refreshing-path-interval", float, rpi, 1);
	//_refresh_waypoints.set(rpi);
}


void AIBuggy::onObstacle(const Object *o) {
/*
	if ((idx % 21) == 1) { //approx once per 5 second
		play_random_sound("klaxon", false);
	}
*/
}

void AIBuggy::calculate(const float dt) {
	ai::Waypoints::calculate(this, dt);

	float rt;
	Config->get("objects." + registered_name + ".rotation-time", rt, 0.05f);
	limit_rotation(dt, rt, true, false);
	update_state_from_velocity();
}

REGISTER_OBJECT("static-buggy", Buggy, ("vehicle"));
REGISTER_OBJECT("buggy", AIBuggy, ("fighting-vehicle"));
