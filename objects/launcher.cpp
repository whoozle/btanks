
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

#include "resource_manager.h"
#include "object.h"
#include "world.h"
#include "game.h"
#include "launcher.h"
#include "config.h"

REGISTER_OBJECT("launcher", Launcher, ());

Launcher::Launcher() 
: Object("player"), _fire(false) {
}

Launcher::Launcher(const std::string &animation) 
: Object("player"), _fire(false) {
	setup(animation);
}

Object * Launcher::clone() const {
	return new Launcher(*this);
}

void Launcher::onSpawn() {
	Object *_smoke = spawnGrouped("single-pose", "launcher-smoke", v3<float>::empty, Centered);
	_smoke->hp = 100000;
	_smoke->impassability = 0;
	add("smoke", _smoke);
	add("mod", spawnGrouped("missiles-on-launcher", "guided-missiles-on-launcher", v3<float>::empty, Centered));
	
	GET_CONFIG_VALUE("objects.launcher.fire-rate", float, fr, 0.3);
	_fire.set(fr);
}


void Launcher::emit(const std::string &event, BaseObject * emitter) {
	if (event == "death") {
		LOG_DEBUG(("dead"));
		cancelAll();
		//play("dead", true);
		spawn("corpse", "dead-" + animation);
		_velocity.x = _velocity.y = _velocity.z = 0;

		Object::emit(event, emitter);
	} else if (event == "launch") {
		v3<float> v = _velocity.is0()?_direction:_velocity;
		v.normalize();
		spawn("guided-missile", "guided-missile", v3<float>::empty, v);
		const Object * la = ResourceManager.get_const()->getAnimation("missile-launch");
		v3<float> dpos = (size - la->size).convert<float>();
		dpos.z = 0;
		dpos /= 2;

		Object *o = spawn("missile-launch", "missile-launch", dpos, _direction);
		o->setDirection(getDirection());
		//LOG_DEBUG(("dir: %d", o->getDirection()));
	} else Object::emit(event, emitter);
}


void Launcher::calculate(const float dt) {
	Object::calculate(dt);
	GET_CONFIG_VALUE("objects.launcher.rotation-time", float, rt, 0.07);
	limitRotation(dt, 8, rt, true, false);
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
}

const bool Launcher::take(const BaseObject *obj, const std::string &type) {
	if (obj->classname == "mod") {
		LOG_DEBUG(("taking mod: %s", type.c_str()));
		remove("mod");
		add("mod", spawnGrouped("machinegunner-on-launcher", "machinegunner-on-launcher", v3<float>::empty, Centered));
		return true;
	}
	if (get("mod")->classname != "missiles-on-launcher" && (obj->classname == "missiles" || obj->classname=="mines")) {
		LOG_DEBUG(("restoring default mod."));
		remove("mod");
		add("mod", spawnGrouped("missiles-on-launcher", "guided-missiles-on-launcher", v3<float>::empty, Centered));
	}
	if (get("mod")->take(obj, type))
		return true;
	return BaseObject::take(obj, type);
}

void Launcher::serialize(mrt::Serializator &s) const {
	Object::serialize(s);
	_fire.serialize(s);
}

void Launcher::deserialize(const mrt::Serializator &s) {
	Object::deserialize(s);
	_fire.deserialize(s);
}
