
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

#include <assert.h>
#include "resource_manager.h"
#include "object.h"
#include "world.h"
#include "game.h"
#include "shilka.h"
#include "config.h"

REGISTER_OBJECT("shilka", Shilka, ());

Shilka::Shilka() 
: Object("player"), _fire(false), _special_fire(false), _left_fire(true) {
}

Shilka::Shilka(const std::string &animation) 
: Object("player"), _fire(false), _special_fire(false), _left_fire(true) {
	setup(animation);
}


void Shilka::onSpawn() {
	Object *_smoke = spawnGrouped("single-pose", "tank-smoke", v3<float>::empty, Centered);
	_smoke->impassability = 0;

	add("smoke", _smoke);
	
	GET_CONFIG_VALUE("objects.shilka.fire-rate", float, fr, 0.2);
	_fire.set(fr);

	GET_CONFIG_VALUE("objects.shilka.special-fire-rate", float, sfr, 0.7);
	_special_fire.set(sfr);
}

Object * Shilka::clone() const {
	return new Shilka(*this);
}


void Shilka::emit(const std::string &event, BaseObject * emitter) {
	if (event == "death") {
		LOG_DEBUG(("dead"));
		cancelAll();
		//play("dead", true);
		spawn("corpse", "dead-" + animation);
		_velocity.x = _velocity.y = _velocity.z = 0;
		Object::emit(event, emitter);
	} else Object::emit(event, emitter);
}


void Shilka::calculate(const float dt) {
	Object::calculate(dt);	
	GET_CONFIG_VALUE("objects.shilka.rotation-time", float, rt, 0.05);
	limitRotation(dt, 8, rt, true, false);

	//LOG_DEBUG(("_velocity: %g %g", _velocity.x, _velocity.y));
}


void Shilka::tick(const float dt) {
	Object::tick(dt);

	bool fire_possible = _fire.tick(dt);
	bool special_fire_possible = _special_fire.tick(dt);
	
	if (getState().empty()) {
		play("hold", true);
	}

	_velocity.normalize();
	if (_velocity.is0()) {
		cancelRepeatable();
		play("hold", true);
	} else {
		if (getState() == "hold") {
			cancelAll();
			play("start", false);
			play("move", true);
		}
	}
	
	

	if (_state.fire && fire_possible) {
		_fire.reset();
		
		if (getState().substr(0,4) == "fire") 
			cancel();
		
		playNow(_left_fire?"fire-left":"fire-right");
		
		static const std::string left_fire = "shilka-bullet-left";
		static const std::string right_fire = "shilka-bullet-right";
		std::string animation = "shilka-bullet-";
		animation += (_left_fire)?"left":"right";
		if (isEffectActive("dispersion")) {
			spawn("dispersion-bullet", "dispersion-bullet", v3<float>::empty, _direction);
		} else
			spawn("shilka-bullet", animation, v3<float>::empty, _direction);
		_left_fire = ! _left_fire;
	}

	if (_state.alt_fire && special_fire_possible && isEffectActive("dirt")) {
		_special_fire.reset();
		if (getState().substr(0,4) == "fire") 
			cancel();
		
		playNow(_left_fire?"fire-left":"fire-right");
		
		static const std::string left_fire = "shilka-bullet-left";
		static const std::string right_fire = "shilka-bullet-right";
		std::string animation = "shilka-dirt-bullet-";
		animation += (_left_fire)?"left":"right";

		spawn("dirt-bullet", animation, v3<float>::empty, _direction);

		_left_fire = ! _left_fire;
	}
}

const bool Shilka::take(const BaseObject *obj, const std::string &type) {
	if (obj->classname == "effects") {
		addEffect(type);
		return true;
	}
	return BaseObject::take(obj, type);
}

void Shilka::serialize(mrt::Serializator &s) const {
	Object::serialize(s);
	_fire.serialize(s);
	_special_fire.serialize(s);
	s.add(_left_fire);
}
void Shilka::deserialize(const mrt::Serializator &s) {
	Object::deserialize(s);
	_fire.deserialize(s);
	_special_fire.deserialize(s);
	s.get(_left_fire);
}
