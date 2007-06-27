
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
#include "object.h"
#include "launcher.h"
#include "config.h"


void Launcher::getImpassabilityPenalty(const float impassability, float &base, float &base_value, float &penalty) const {
	if (impassability > 0.2) {
		base_value = 0.2;
		base = 0.2;
		penalty = 1.5;
	}
}


Launcher::Launcher(const std::string &classname) 
: Object(classname), _fire(false) {
}

Object * Launcher::clone() const {
	return new Launcher(*this);
}

void Launcher::onSpawn() {
	if (registered_name.substr(0, 6) == "static")
		disown();

	Object *_smoke = spawnGrouped("single-pose", "launcher-smoke", v2<float>(), Centered);
	_smoke->hp = 100000;
	_smoke->impassability = 0;
	add("smoke", _smoke);
	add("mod", spawnGrouped("missiles-on-launcher", "guided-missiles-on-launcher", v2<float>(), Centered));
	add("alt-mod", spawnGrouped("alt-missiles-on-launcher", "guided-missiles-on-launcher", v2<float>(2,2), Centered));
	
	GET_CONFIG_VALUE("objects.launcher.fire-rate", float, fr, 0.3);
	_fire.set(fr);
	play("hold", true);
}


void Launcher::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
		LOG_DEBUG(("dead"));
		if (disable_ai)
			detachVehicle();
		
		spawn("corpse", "dead-" + animation);
		Object::emit(event, emitter);
	} else Object::emit(event, emitter);
}


void Launcher::calculate(const float dt) {
	Object::calculate(dt);
	GET_CONFIG_VALUE("objects.launcher.rotation-time", float, rt, 0.07);
	limitRotation(dt, rt, true, false);
	//LOG_DEBUG(("_velocity: %g %g", _velocity.x, _velocity.y));
}


void Launcher::tick(const float dt) {
	Object::tick(dt);

	bool fire_possible = _fire.tick(dt);
	
	if (getState().empty()) {
		play("hold", true);
		groupEmit("mod", "hold");
	}

	if (_velocity.is0()) {	
		cancelRepeatable();
		play("hold", true);
		groupEmit("mod", "hold");
	} else {
		if (getState() == "hold") {
			cancelAll();
			//play("start", false);
			play("move", true);
			groupEmit("mod", "move");
		}
	}

	if (_state.fire && fire_possible) {
		_fire.reset();
		groupEmit("mod", "launch");
	}
	if (_state.alt_fire && fire_possible) {
		_fire.reset();
		groupEmit("alt-mod", "launch");
	}
}

const bool Launcher::take(const BaseObject *obj, const std::string &type) {
	if (BaseObject::take(obj, type)) 
		return true;

	if (obj->classname == "mod" && type == "machinegunner") {
		LOG_DEBUG(("taking mod: %s", type.c_str()));
		remove("mod");
		add("mod", spawnGrouped("machinegunner-on-launcher", "machinegunner-on-launcher", v2<float>(), Centered));
		return true;
	}
	const bool primary_mod = (obj->classname == "missiles" && (type != "smoke" && type != "stun" && type != "nuke"));
	if (primary_mod && get("mod")->classname != "missiles-on-vehicle") {
		LOG_DEBUG(("restoring default mod."));
		remove("mod");
		add("mod", spawnGrouped("missiles-on-launcher", "guided-missiles-on-launcher", v2<float>(), Centered));
	}
	if (primary_mod) {
		if (get("mod")->take(obj, type))
			return true;
	} else {
		if (get("alt-mod")->take(obj, type))
			return true;		
	}
	
	return false;
}

void Launcher::serialize(mrt::Serializator &s) const {
	Object::serialize(s);
	s.add(_fire);
}

void Launcher::deserialize(const mrt::Serializator &s) {
	Object::deserialize(s);
	if (registered_name == "static-launcher")
		_state.clear();
	s.get(_fire);
}

REGISTER_OBJECT("launcher", Launcher, ("fighting-vehicle"));
REGISTER_OBJECT("static-launcher", Launcher, ("vehicle"));
