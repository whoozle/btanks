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

#include "object.h"
#include "registrar.h"
#include "ai/herd.h"
#include "ai/trooper.h"
#include "ai/targets.h"
#include "config.h"

class Slime : public Object, private ai::StupidTrooper, private ai::Herd {
public: 
	Slime() : Object("monster"), ai::StupidTrooper("slime-acid", ai::Targets->monster), _fire(false) {}
	Object *clone() const { return new Slime(*this); }

	void get_impassability_penalty(const float impassability, float &base, float &base_value, float &penalty) const {
		if (impassability > 0.2f) {
			base_value = 0.2f;
			base = 0;
			penalty = 0;
		}
	}

	const bool take(const BaseObject *obj, const std::string &type) {
		return false;
	}

	void emit(const std::string &event, Object * emitter) {
		if (event == "death") {
			spawn("corpse(slime-death)", "dead-" + animation);
		}
		Object::emit(event, emitter);
	}
	
	const int getComfortDistance(const Object *other) const {
		GET_CONFIG_VALUE("objects.slime.comfort-distance", int, cd, 120);
		return (other == NULL || other->classname == classname)?cd:-1; //fixme names if you want
	}
	
	void onIdle() {
		_state.fire = false;

		float tt = getWeaponRange("slime-acid");
		ai::Herd::calculateV(_velocity, this, 0, tt);
	}

	void tick(const float dt);
	void calculate(const float dt);
	
	void on_spawn() {
		disown();
		play("hold", true);
		ai::StupidTrooper::on_spawn();
		//GET_CONFIG_VALUE("objects.slime.fire-rate", float, fr, 1.0f);
		float fr = 1.0f;
		_fire.set(fr);
	}
	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		ai::StupidTrooper::serialize(s);
		s.add(_fire);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		ai::StupidTrooper::deserialize(s);
		s.get(_fire);	
	}
private: 
	Alarm _fire;
};

void Slime::calculate(const float dt) {
	ai::StupidTrooper::calculate(this, _state, _velocity, _direction, dt);

	GET_CONFIG_VALUE("objects.slime.rotation-time", float, rt, 0.2);
	limit_rotation(dt, rt, true, false);	
}

void Slime::tick(const float dt) {
	Object::tick(dt);

	const std::string state = get_state();
	
	if (_velocity.is0() && state == "move") {
		cancel_all();
		play("hold", true);
	} else if (!_velocity.is0() && state == "hold") {
		cancel_all();
		play("move", true);
	}
	if (_fire.tick(dt) && _state.fire) {
		_fire.reset();
		spawn("slime-acid", "slime-acid", v2<float>(), _direction);
		if (state != "fire") 
			play_now("fire");
	}
}

REGISTER_OBJECT("slime", Slime, ());
