
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
#include "object.h"
#include "launcher.h"
#include "config.h"
#include "special_owners.h"

void Launcher::get_impassability_penalty(const float impassability, float &base, float &base_value, float &penalty) const {
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

void Launcher::on_spawn() {
	if (registered_name.substr(0, 6) == "static") {
		remove_owner(OWNER_MAP);
		disable_ai = true;
	}

	Object *_smoke = add("smoke", "single-pose", "launcher-smoke", v2<float>(), Centered);
	_smoke->hp = 100000;
	_smoke->impassability = 0;
	std::string default_mod;
	Config->get("objects.launcher.default-mod", default_mod, "missiles-on-launcher");
	if (default_mod == "missiles-on-launcher") 
		add("mod", "missiles-on-launcher", "guided-missiles-on-launcher", v2<float>(), Centered);
	else if (default_mod == "machinegunner-on-launcher")
		add("mod", "machinegunner-on-launcher", "machinegunner-on-launcher", v2<float>(), Centered);
	else if (default_mod == "thrower-on-launcher")
		add("mod", "thrower-on-launcher", "thrower-on-launcher", v2<float>(), Centered);
	else
		throw_ex(("unknown mod type %s", default_mod.c_str()));
	
	add("alt-mod", "alt-missiles-on-launcher", "guided-missiles-on-launcher", v2<float>(2,2), Centered);
	
	GET_CONFIG_VALUE("objects.launcher.fire-rate", float, fr, 0.3);
	_fire.set(fr);
	play("hold", true);
}

void Launcher::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
		spawn("corpse", "dead-" + animation);
		_dead = true;
		detachVehicle();
		Object::emit(event, emitter);
	} else 
		Object::emit(event, emitter);
}


void Launcher::calculate(const float dt) {
	Object::calculate(dt);
	GET_CONFIG_VALUE("objects.launcher.rotation-time", float, rt, 0.07);
	limit_rotation(dt, rt, true, false);
	//LOG_DEBUG(("_velocity: %g %g", _velocity.x, _velocity.y));
}


void Launcher::tick(const float dt) {
	Object::tick(dt);
	if (!playing_sound("vehicle-sound")) {
		play_sound("vehicle-sound", true, 0.4f);
	}

	bool fire_possible = _fire.tick(dt);
	
	if (get_state().empty()) {
		play("hold", true);
		group_emit("mod", "hold");
	}

	if (_velocity.is0()) {	
		cancel_repeatable();
		play("hold", true);
		group_emit("mod", "hold");
	} else {
		if (get_state() == "hold") {
			cancel_all();
			//play("start", false);
			play("move", true);
			group_emit("mod", "move");
		}
	}

	if (_state.fire && fire_possible) {
		_fire.reset();
		group_emit("mod", "launch");
	}
	if (_state.alt_fire && fire_possible) {
		_fire.reset();
		group_emit("alt-mod", "launch");
	}
}

const bool Launcher::take(const BaseObject *obj, const std::string &type) {
	if (Object::take(obj, type)) 
		return true;

	if (obj->classname == "mod" && (type == "machinegunner" || type == "thrower")) {
		std::string mod_name = type + "-on-launcher";
		if (get("mod")->registered_name == mod_name) 
			return false;
		
		LOG_DEBUG(("taking mod: %s", type.c_str()));
		remove("mod");
		add("mod", mod_name, mod_name, v2<float>(), Centered);
		return true;
	}
	const bool primary_mod = (obj->classname == "missiles" && (type != "smoke" && type != "stun" && type != "nuke"));
	if (primary_mod && get("mod")->classname != "missiles-on-vehicle") {
		LOG_DEBUG(("restoring default mod."));
		remove("mod");
		add("mod", "missiles-on-launcher", "guided-missiles-on-launcher", v2<float>(), Centered);
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
	s.get(_fire);
}

