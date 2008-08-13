
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
#include "registrar.h"
#include "resource_manager.h"
#include "alarm.h"
#include "config.h"
#include "trooper.h"
#include "game_monitor.h"
#include "rt_config.h"
#include <string.h>

void Trooper::get_impassability_penalty(const float impassability, float &base, float &base_value, float &penalty) const {
	if (impassability > 0.2f) {
		base_value = 0.2f;
		base = 0;
		penalty = 0;
	}
}

void Trooper::tick(const float dt) {
	set_direction(_velocity.get_direction8() - 1);
	Object::tick(dt);
	
	if (!_state.fire) {
		if (get_state() == "fire")
			cancel_all();
	}
	
	if (_velocity.is0()) {
		const std::string state = get_state();
		if (state != "hold" && state != "fire" && state != "throw") {
			cancel_all();
			play("hold", true);
			if (has("helmet")) {
				Object *helmet = get("helmet");
				helmet->cancel_all();
				helmet->play("hold", true);
			}
		}
	} else {
		const std::string state = get_state();
		if (state == "hold" || state.empty()) {
			cancel_all();
			play("run", true);
			if (has("helmet")) {
				Object *helmet = get("helmet");
				helmet->cancel_all();
				helmet->play("run", true);
			}
		}		
	}
	
	if (!_object.empty() && _fire.tick(dt) && _state.fire && !_variants.has("nukeman")) {
		_fire.reset();
		if (disable_ai || validateFire(0)) {
			if (get_state() != "fire") {
				cancel_all();
				play("fire", true);
			}
			spawn(_object, _object, v2<float>(), _direction);
		}
	}
	if (_alt_fire.tick(dt) && _state.alt_fire) {
		_alt_fire.reset();
		if (_variants.has("nukeman")) {
			//MUGGAGAGAGAGAG!!!
			Object *o = spawn("nuke-explosion", "nuke-explosion");
			emit("death", o); 
		} else if (!_variants.has("no-grenades")) {
			if (get_state() != "throw")
				play_now("throw");
			spawn("grenade", "grenade", v2<float>(), _direction);
		}
	}
}

const bool Trooper::take(const BaseObject *obj, const std::string &type) {
	if (obj->classname == "missiles" && type == "nuke" && _variants.has("player") && !_variants.has("nukeman")) {
		if (GameMonitor->getCampaign() != NULL || RTConfig->game_type == GameTypeCTF) 
			return Object::take(obj, type);

		_variants.add("nukeman");
		hp = max_hp = 999;
		init("nukeman");
		invalidate();
		return true;
	}
	return Object::take(obj, type);
}

#include "world.h"

void Trooper::on_spawn() {
	if (_variants.has("player")) {
		speed *= 1.75f;
		hp = max_hp *= 2;
	}

	int sid = get_summoner();
	const Object *summoner = World->getObjectByID(sid);
	if (summoner != NULL) {
		const std::string &a = summoner->animation;
		static const char *colors[4] = {"red-", "green-", "yellow-", "blue-"};
		int i;
		for(i = 0; i < 4; ++i) {
			size_t l = strlen(colors[i]);
			if (a.size() > l && a.compare(0, l, colors[i]) == 0)
				break;
		}
		if (i < 4) {
			std::string animation = colors[i] + registered_name + "-helmet";
			//LOG_DEBUG(("helmet animation = %s", animation.c_str()));
			if (ResourceManager->hasAnimation(animation)) {
				add("helmet", "helmet", animation, v2<float>(), Centered);
			}
		}
	}
	
	if (_variants.has("disembark")) {
		play_sound("disembark", false);
		//add helmet if parent player detected.
	}
	GET_CONFIG_VALUE("objects.trooper.grenade-rate", float, gr, 1.2f);
	_alt_fire.set(gr);
	
	if (_object.empty()) {
		//nothing to do
	} else if (_object == "thrower-missile") {
		GET_CONFIG_VALUE("objects.thrower.fire-rate", float, fr, 3);
		_fire.set(fr);
	} else if (_object == "machinegunner-bullet") {
		GET_CONFIG_VALUE("objects.machinegunner.fire-rate", float, fr, 0.2);
		_fire.set(fr);
	} else throw_ex(("unsupported weapon %s", _object.c_str()));
	
	play("hold", true);
	_pose = "run";
}

void Trooper::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
		spawn("corpse(human-death)", "dead-" + animation, v2<float>(), v2<float>());
	} else if (event == "collision" && emitter != NULL && emitter->classname == "vehicle" && !_variants.has("nukeman")) {
		if (
			(
				(disable_ai && _velocity.same_sign(get_relative_position(emitter))) || 
				(registered_name == "machinegunner-player")
			) && attachVehicle(emitter))
			return;
	}
	Object::emit(event, emitter);
}

const bool Trooper::validateFire(const int idx) {
	return true;
}

Object* Trooper::clone() const  {
	return new Trooper(*this);
}
