
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

#define _USE_MATH_DEFINES
#include <math.h>
#include <assert.h>
#include "registrar.h"
#include "object.h"
#include "shilka.h"
#include "config.h"
#include "fakemod.h"
#include "rt_config.h"
#include "special_owners.h"

void Shilka::get_impassability_penalty(const float impassability, float &base, float &base_value, float &penalty) const {
	base = 0;
	base_value = 0;
	penalty = 0.8;
}

Shilka::Shilka(const std::string &classname) 
: Object(classname), _special_fire(false), _left_fire(true) {
}

FakeMod *Shilka::getMod(const std::string &name) {
	Object *o = get(name);
	assert(o != NULL);
	FakeMod *f = dynamic_cast<FakeMod*>(o);
	if (f == NULL)
		throw_ex(("cannot get FakeMod instance. [got %s(%s)]", o->registered_name.c_str(), o->classname.c_str()));
	return f;
}

void Shilka::on_spawn() {
	if (registered_name.substr(0, 6) == "static") {
		remove_owner(OWNER_MAP);
		disable_ai = true;
	}
	
	add("mod", "shilka-turret", animation + "-turret", v2<float>(), Centered);
	add("alt-mod", "fake-mod", "damage-digits", v2<float>(), Centered);
	
	Object *_smoke = add("smoke", "single-pose", "tank-smoke", v2<float>(), Centered);
	_smoke->impassability = 0;
	
	GET_CONFIG_VALUE("objects.shilka.special-fire-rate", float, sfr, 0.4f);
	_special_fire.set(sfr);
	play("hold", true);
	play_sound("vehicle-sound", true, 0.4f);
}

Object * Shilka::clone() const {
	return new Shilka(*this);
}

#include "math/binary.h"

void Shilka::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
		LOG_DEBUG(("dead"));
		cancel_all();
		//play("dead", true);
		Object *corpse = spawn("corpse", "dead-" + animation);
		
		FakeMod *mod = getMod("alt-mod");
		std::string mod_type = mod->getType();
		//LOG_DEBUG(("mod: %s", mod_type.c_str()));
		if (mod_type == "thrower" || mod_type == "machinegunner") {
			int max;
			Config->get("objects.shilka.units-limit", max, 10); 
			
			int n = mod->getCount(), now = get_children("trooper");
			if (n + now > max) {
				n = max - now;
			}
			for(int i = 0; i < n; ++i) {
				spawn(mod_type + "(disembark)" + ((RTConfig->game_type == GameTypeCooperative && get_slot() >= 0)? "(ally)":""), 
					mod_type, 
					v2<float>(size.x * cos(M_PI * 2 * i / n), size.y * sin(M_PI * 2 * i / n)), 
					v2<float>());
			}
		} else if (mod_type == "mines:nuke") {
			Object *mine = spawn("nuke-mine", "nuke-mine", v2<float>(), v2<float>());
			mine->set_z(corpse->get_z() + 1, true);
		}
		
		_dead = true;
		detachVehicle();
		Object::emit(event, emitter);
	} else 
		Object::emit(event, emitter);
}


void Shilka::calculate(const float dt) {
	Object::calculate(dt);	
	GET_CONFIG_VALUE("objects.shilka.rotation-time", float, rt, 0.05);
	limit_rotation(dt, rt, true, false);

	//LOG_DEBUG(("_velocity: %g %g", _velocity.x, _velocity.y));
}


void Shilka::tick(const float dt) {
	if (get_state().empty()) {
		play("hold", true);
	}

	Object::tick(dt);
	{
		PlayerState state = _state;
		state.left = 0;
		state.right = 0;
		state.up = 0;
		state.down = 0;
		get("mod")->update_player_state(state);
	}
		

	const bool special_fire_possible = _special_fire.tick(dt);
	

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
	
	if (_state.alt_fire && special_fire_possible) {
		_special_fire.reset();
		
		FakeMod * mod = getMod("alt-mod");
		std::string mod_type = mod->getType();
		
		if (mod_type.substr(0, 6) == "mines:") {
			std::vector<std::string> res;
			mrt::split(res, mod_type, ":", 2);
			res[0].resize(res[0].size() - 1);
			std::string name = res[1] + "-" + res[0];
			if (mod->getCount() > 0) {
				spawn(name, name, _direction*(size.length()/-2), v2<float>());
				mod->decreaseCount();
			}
			
		} else if (!mod_type.empty()) {
			int n;
			Config->get("objects.shilka.units-limit", n, 10); //fixme: add type restrictions
			if (mod->getCount() > 0 && get_children("trooper") < n) {
				spawn(mod_type + "(disembark)" + (RTConfig->game_type == GameTypeCooperative? "(ally)":""), mod_type, _direction*(size.length()/-2), v2<float>());
				mod->decreaseCount();
			}
		}
	}
}

const bool Shilka::take(const BaseObject *obj, const std::string &type) {
	if (Object::take(obj, type))
		return true;
	
	if (obj->classname == "effects") {
		if (type == "dispersion") {
			remove_effect("ricochet");
		} else if (type == "ricochet") {
			remove_effect("dispersion");
		} else if (type == "dirt") {
			getMod("alt-mod")->setType(std::string());
		}
		add_effect(type);
		return true;
	} else if (obj->classname =="mod") {
		if (type == "machinegunner" || type == "thrower") {
			remove_effect("dirt");
			FakeMod *mod = getMod("alt-mod");

			int n;
			Config->get("objects.shilka." + type + "-capacity", n, 5);
			if (mod->getCount() >= n && type == mod->getType()) 
				return false;

			mod->setType(type);
			mod->setCount(n);
			return true;
		}
	} else if (obj->classname == "mines") {
		remove_effect("dirt");
		FakeMod *mod = getMod("alt-mod");
		int n;
		Config->get("objects.shilka." + type + "-" + obj->classname + "-capacity", n, 7);
		
		if (mod->getCount() >= n && mod->getType() == obj->classname + ":" + type)
			return false;
		
		mod->setType(obj->classname + ":" + type);
		mod->setCount(n);
		return true;		
	} else if (obj->classname == "missiles" && type == "nuke") {
		remove_effect("dirt");
		FakeMod *mod = getMod("alt-mod");
		int n;
		Config->get("objects.shilka.nuke-mines-capacity", n, 3);

		if (mod->getCount() >= n && mod->getType() == "mines:nuke")
			return false;

		mod->setType("mines:nuke");
		mod->setCount(n);
		return true;
	}
	return false;
}

void Shilka::serialize(mrt::Serializator &s) const {
	Object::serialize(s);
	s.add(_special_fire);
}
void Shilka::deserialize(const mrt::Serializator &s) {
	Object::deserialize(s);
	s.get(_special_fire);
	if (!playing_sound("vehicle-sound"))
		play_sound("vehicle-sound", true, 0.4f);
}
