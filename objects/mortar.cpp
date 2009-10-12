
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

#include <assert.h>
#include "registrar.h"
#include "object.h"
#include "mortar.h"
#include "config.h"
#include "special_owners.h"

void Mortar::get_impassability_penalty(const float impassability, float &base, float &base_value, float &penalty) const {
	penalty = 0;
	base_value = 0;
	base = 0;
}

Mortar::Mortar(const std::string &classname) 
: Object(classname), _fire(false) {
}

void Mortar::on_spawn() {
	if (registered_name.substr(0, 6) == "static") {
		disable_ai = true;
		remove_owner(OWNER_MAP);
	}

	GET_CONFIG_VALUE("objects.mortar.fire-rate", float, fr, 0.7);
	_fire.set(fr);
	play("hold", true);
}

Object * Mortar::clone() const {
	return new Mortar(*this);
}

void Mortar::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
		spawn("corpse", "dead-mortar");
		_velocity.clear();
		_dead = true;
		detachVehicle();
		Object::emit(event, emitter);
	} else 
		Object::emit(event, emitter);
}

const bool Mortar::take(const BaseObject *obj, const std::string &type) {
	if (Object::take(obj, type))
		return true;
	//custom code
	return false;
}

void Mortar::calculate(const float dt) {
	Object::calculate(dt);
	GET_CONFIG_VALUE("objects.mortar.rotation-time", float, rt, 0.2);
	limit_rotation(dt, rt, true, false);
}

void Mortar::tick(const float dt) {
	if (get_state().empty()) {
		play("hold", true);
	}

	Object::tick(dt);

	if (!playing_sound("vehicle-sound")) {
		play_sound("vehicle-sound", true, 0.4f);
	}

	bool fire_possible = _fire.tick(dt);
	
	_velocity.normalize();
	if (_velocity.is0()) {
		cancel_repeatable();
		play("hold", true);
	} else {
		if (get_state() == "hold") {
			cancel_all();
			play("move", true);
		}
	}


	if (_state.fire && fire_possible) {
		_fire.reset();
		/*
		if (get_state() == "fire") 
			cancel();
		
		play_now("fire");
		*/
		spawn("mortar-bullet", "mortar-bullet", v2<float>(), _direction);
	}
}

void Mortar::serialize(mrt::Serializator &s) const {
	Object::serialize(s);
	s.add(_fire);
}

void Mortar::deserialize(const mrt::Serializator &s) {
	Object::deserialize(s);
	s.get(_fire);
}

