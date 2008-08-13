
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

#include <assert.h>
#include "registrar.h"
#include "object.h"
#include "tank.h"
#include "config.h"
#include "special_owners.h"

void Tank::get_impassability_penalty(const float impassability, float &base, float &base_value, float &penalty) const {
	if (impassability > 0.4) {
		penalty = 0;
		base_value = 0.3;
	} else {
		penalty = 0.3/0.4;
	}
}

Tank::Tank(const std::string &classname) 
: Object(classname), _fire(false) {
}

void Tank::on_spawn() {
	if (registered_name.substr(0, 6) == "static") {
		remove_owner(OWNER_MAP);
		disable_ai = true;
	}

	Object *_smoke = add("smoke", "single-pose", "tank-smoke", v2<float>(), Centered);
	_smoke->impassability = 0;

	Object *_missiles = add("mod", "missiles-on-tank", "guided-missiles-on-tank", v2<float>(), Centered);
	_missiles->impassability = 0;
	
	GET_CONFIG_VALUE("objects.tank.fire-rate", float, fr, 0.3);
	_fire.set(fr);
	play("hold", true);
	play_sound("vehicle-sound", true, 0.4f);
}

Object * Tank::clone() const {
	return new Tank(*this);
}

void Tank::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
		cancel_all();
		spawn("corpse", "dead-" + animation);
		_velocity.clear();
		_dead = true;
		detachVehicle();
		Object::emit(event, emitter);
	} else 
		Object::emit(event, emitter);
}

const bool Tank::take(const BaseObject *obj, const std::string &type) {
	if (Object::take(obj, type))
		return true;
	
	if (obj->classname == "effects") {
		float def = 10;
		if (type == "dispersion") {
			def = -1;
			remove_effect("dirt");
			remove_effect("ricochet");
		} else if (type == "ricochet") {
			def = 60;
			remove_effect("dirt");
			remove_effect("dispersion");
		}
		float d;
		Config->get("objects.tank." + type + ".duration", d, def);
		add_effect(type, d);
		return true;
	}
	if (get("mod")->take(obj, type))
		return true;
	return false;
}

void Tank::calculate(const float dt) {
	Object::calculate(dt);
	GET_CONFIG_VALUE("objects.tank.rotation-time", float, rt, 0.05);
	limit_rotation(dt, rt, true, false);
}

void Tank::tick(const float dt) {
	if (get_state().empty()) {
		play("hold", true);
	}

	Object::tick(dt);

	bool fire_possible = _fire.tick(dt);
	_velocity.normalize();
	if (_velocity.is0()) {
		cancel_repeatable();
		play("hold", true);
		group_emit("mod", "hold");
	} else {
		if (get_state() == "hold") {
			cancel_all();
			play("start", false);
			play("move", true);
			group_emit("mod", "move");
		}
	}


	if (_state.fire && fire_possible) {
		_fire.reset();
		
		if (get_state() == "fire") 
			cancel();
		
		play_now("fire");
		
		//LOG_DEBUG(("vel: %f %f", _state.old_vx, _state.old_vy));
		//v2<float> v = _velocity.is0()?_direction:_velocity;
		//v.normalize();
		std::string bullet("tank-bullet");
		std::string var;
		if (has_effect("dirt")) {
			bullet = "dirt-bullet";
		} else if (has_effect("dispersion")) {
			bullet = "dispersion-bullet";
		} else if (has_effect("ricochet")) {
			bullet = "ricochet-bullet";
			var = "(auto-aim)";
		}
		spawn(bullet + var, bullet, v2<float>(), _direction);
	}
	if (_state.alt_fire && fire_possible) {
		_fire.reset();
		group_emit("mod", "launch");
	}
	
	//LOG_DEBUG(("_velocity: %g %g", _velocity.x, _velocity.y));
}

void Tank::serialize(mrt::Serializator &s) const {
	Object::serialize(s);
	s.add(_fire);
}
void Tank::deserialize(const mrt::Serializator &s) {
	Object::deserialize(s);
	s.get(_fire);
	if (!playing_sound("vehicle-sound"))
		play_sound("vehicle-sound", true, 0.4f);
}
