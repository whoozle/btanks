
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
#include "world.h"
#include "tank.h"
#include "config.h"

void Tank::getImpassabilityPenalty(const float impassability, float &base, float &base_value, float &penalty) const {
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

void Tank::onSpawn() {
	if (registered_name.substr(0, 6) == "static")
		disown();

	Object *_smoke = spawnGrouped("single-pose", "tank-smoke", v2<float>::empty, Centered);
	_smoke->impassability = 0;

	Object *_missiles = spawnGrouped("missiles-on-tank", "guided-missiles-on-tank", v2<float>::empty, Centered);
	_missiles->impassability = 0;

	add("mod", _missiles);
	add("smoke", _smoke);
	
	GET_CONFIG_VALUE("objects.tank.fire-rate", float, fr, 0.3);
	_fire.set(fr);
}

Object * Tank::clone() const {
	return new Tank(*this);
}

void Tank::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
		LOG_DEBUG(("dead"));
		cancelAll();
		//play("dead", true);
		if (registered_name != "ai-tank")
			World->detachVehicle(this);
		spawn("corpse", "dead-" + animation);
		_velocity.clear();
		Object::emit(event, emitter);
	} else Object::emit(event, emitter);
}

const bool Tank::take(const BaseObject *obj, const std::string &type) {
	if (obj->classname == "effects") {
		float def = 10;
		if (type == "dispersion") {
			def = -1;
			removeEffect("dirt");
			removeEffect("ricochet");
		} else if (type == "ricochet") {
			def = 60;
			removeEffect("dirt");
			removeEffect("dispersion");
		}
		float d;
		Config->get("objects.tank." + type + ".duration", d, def);
		addEffect(type, d);
		return true;
	}
	if (get("mod")->take(obj, type))
		return true;
	return BaseObject::take(obj, type);
}

void Tank::calculate(const float dt) {
	Object::calculate(dt);
	GET_CONFIG_VALUE("objects.tank.rotation-time", float, rt, 0.05);
	limitRotation(dt, rt, true, false);
}

void Tank::tick(const float dt) {
	Object::tick(dt);

	bool fire_possible = _fire.tick(dt);
	
	if (getState().empty()) {
		play("hold", true);
	}

	_velocity.normalize();
	if (_velocity.is0()) {
		cancelRepeatable();
		play("hold", true);
		groupEmit("mod", "hold");
	} else {
		if (getState() == "hold") {
			cancelAll();
			play("start", false);
			play("move", true);
			groupEmit("mod", "move");
		}
	}


	if (_state.fire && fire_possible) {
		_fire.reset();
		
		if (getState() == "fire") 
			cancel();
		
		playNow("fire");
		
		//LOG_DEBUG(("vel: %f %f", _state.old_vx, _state.old_vy));
		//v2<float> v = _velocity.is0()?_direction:_velocity;
		//v.normalize();
		std::string bullet("bullet");
		if (isEffectActive("dirt")) {
			bullet = "dirt-bullet";
		} else if (isEffectActive("dispersion")) {
			bullet = "dispersion-bullet";
		} else if (isEffectActive("ricochet")) {
			bullet = "ricochet-bullet";
		}
		spawn(bullet, bullet, v2<float>::empty, _direction);
	}
	if (_state.alt_fire && fire_possible) {
		_fire.reset();
		groupEmit("mod", "launch");
	}
	
	//LOG_DEBUG(("_velocity: %g %g", _velocity.x, _velocity.y));
}

void Tank::serialize(mrt::Serializator &s) const {
	Object::serialize(s);
	_fire.serialize(s);
}
void Tank::deserialize(const mrt::Serializator &s) {
	Object::deserialize(s);
	if (registered_name == "static-tank")
		_state.clear();
	_fire.deserialize(s);
}

REGISTER_OBJECT("tank", Tank, ("player"));
REGISTER_OBJECT("static-tank", Tank, ("vehicle"));
