
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

#include <assert.h>
#include "resource_manager.h"
#include "object.h"
#include "mortar.h"
#include "config.h"

void Mortar::getImpassabilityPenalty(const float impassability, float &base, float &base_value, float &penalty) const {
	penalty = 0;
	base_value = 0;
	base = 0;
}

Mortar::Mortar(const std::string &classname) 
: Object(classname), _fire(false) {
}

void Mortar::onSpawn() {
	if (registered_name.substr(0, 6) == "static")
		disown();

	GET_CONFIG_VALUE("objects.mortar.fire-rate", float, fr, 0.7);
	_fire.set(fr);
	play("hold", true);
}

Object * Mortar::clone() const {
	return new Mortar(*this);
}

void Mortar::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
		if (disable_ai)
			detachVehicle();
		spawn("corpse", "dead-mortar");
		_velocity.clear();
		Object::emit(event, emitter);
	} else Object::emit(event, emitter);
}

const bool Mortar::take(const BaseObject *obj, const std::string &type) {
	return BaseObject::take(obj, type);
}

void Mortar::calculate(const float dt) {
	Object::calculate(dt);
	GET_CONFIG_VALUE("objects.mortar.rotation-time", float, rt, 0.2);
	limitRotation(dt, rt, true, false);
}

void Mortar::tick(const float dt) {
	if (getState().empty()) {
		play("hold", true);
	}

	Object::tick(dt);

	bool fire_possible = _fire.tick(dt);
	
	_velocity.normalize();
	if (_velocity.is0()) {
		cancelRepeatable();
		play("hold", true);
	} else {
		if (getState() == "hold") {
			cancelAll();
			play("move", true);
		}
	}


	if (_state.fire && fire_possible) {
		_fire.reset();
		/*
		if (getState() == "fire") 
			cancel();
		
		playNow("fire");
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
	if (registered_name == "static-mortar")
		_state.clear();
	s.get(_fire);
}

REGISTER_OBJECT("mortar", Mortar, ("fighting-vehicle"));
REGISTER_OBJECT("static-mortar", Mortar, ("vehicle"));
