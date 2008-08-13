/* Battle Tanks Game
 * Copyright (C) 2006-2008 Battle Tanks team
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

#include "object.h"
#include "alarm.h"
#include "registrar.h"
#include "mrt/random.h"
#include "tmx/map.h"
#include "game_monitor.h"
#include "world.h"

class Wagon : public Object {
public: 
	Wagon() : Object("train") { set_directions_number(1); }
	virtual void on_spawn() { 
		play("move", true); 
		disown(); 
	
		Object *o = World->getObjectByID(get_summoner());
		if (o == NULL) {
			emit("death", NULL);
			return;
		}
		add_owner(o->get_id());	
	}	
	virtual Object * clone() const { return new Wagon(*this); }
	virtual void calculate(const float dt) {
		Object *o = World->getObjectByID(get_summoner());
		if (o == NULL) {
			emit("death", NULL);
			return;
		}
		_velocity = get_relative_position(o);
		float l = _velocity.normalize();
		//LOG_DEBUG(("velocity: %g,%g (%g) (%g)", _velocity.x, _velocity.y, l, 1.2f * size.y));
		if (l < 1.0f * size.y || l > 1.2f * size.y)
			_velocity.clear(); //too close or far
	}
	virtual void emit(const std::string &event, Object * emitter = NULL) {
		if (event == "death") {
			spawn("impassable-corpse", "dead-choo-choo-wagon");
		}
		Object::emit(event, emitter);
	}
	

	void get_impassability_penalty(const float impassability, float &base, float &base_value, float &penalty) const {
		base = base_value = penalty = 0;
	}
};

class Train : public Object {
public:
	Train() : Object("train"), _smoke(1.0, true), _wagon_id(0) { set_directions_number(1); }
	virtual Object * clone() const;
	virtual void on_spawn();
	virtual void calculate(const float dt);
	virtual void tick(const float dt);
	virtual void emit(const std::string &event, Object * emitter = NULL);

	void get_impassability_penalty(const float impassability, float &base, float &base_value, float &penalty) const {
		base = base_value = penalty = 0;
	}

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(dst_y);
		s.add(_smoke);
		s.add(_wagon_id);
	}

	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(dst_y);
		s.get(_smoke);
		s.get(_wagon_id);
	}

private: 
	int dst_y;
	Alarm _smoke;
	int _wagon_id;
};

void Train::on_spawn() {
	play("move", true);
	v2<int> map_size = Map->get_size();
	dst_y = map_size.y - (int)(size.y) / 2 - 4; //fixme. :)
	disown();

	if (_variants.has("standing"))
		classname = "destructable-object";
}

void Train::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
		Object * o = spawn("impassable-corpse", "dead-choo-choo-train");
		o->impassability = 1;
	}
	Object::emit(event, emitter);
}

void Train::tick(const float dt) {
	Object::tick(dt);
	if (Map->torus()) {
		if (!_wagon_id) {
			_wagon_id = spawn("choo-choo-wagon", "choo-choo-wagon", v2<float>(0, -size.y))->get_id();
		}
	} else { 
		v2<int> pos;
		get_position(pos);
		if (pos.y >= 0 && !_wagon_id) {
			_wagon_id = spawn("choo-choo-wagon", "choo-choo-wagon", v2<float>(0, -size.y))->get_id();
		}
		if (pos.y  >= dst_y && !GameMonitor->game_over()) { 
			LOG_DEBUG(("escaped!"));
			if (_variants.has("win-on-exit")) 
				GameMonitor->game_over("messages", "train-saved", 5, true);
			//Object::emit("death", NULL);
		}
	}
	//LOG_DEBUG(("pos: %d dst: %d", pos.y, dst_y));
	if (_smoke.tick(dt)) {
		spawn("train-smoke", "train-smoke");
	}
}

void Train::calculate(const float dt) {
	_state.down = true;
	Object::calculate(dt);
}

Object* Train::clone() const  {
	return new Train(*this);
}

REGISTER_OBJECT("choo-choo-train", Train, ());
REGISTER_OBJECT("choo-choo-wagon", Wagon, ());
