
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

#include "object.h"
#include "resource_manager.h"
#include "alarm.h"
#include "config.h"
#include "world.h"
#include "trooper.h"

class TrooperInWatchTower : public Trooper {
public: 
	TrooperInWatchTower(const std::string &object, const bool aim_missiles) : Trooper("trooper", object, aim_missiles), _reaction(true) {}
	virtual Object * clone() const { return new TrooperInWatchTower(*this); }
	
	virtual void onSpawn() { 
		GET_CONFIG_VALUE("objects.trooper.reaction-time", float, rt, 0.1);
		_reaction.set(rt);
	
		Trooper::onSpawn();
	}

	virtual void serialize(mrt::Serializator &s) const {
		Trooper::serialize(s);
		_reaction.serialize(s);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Trooper::deserialize(s);
		_reaction.deserialize(s);
	}
	
	virtual void calculate(const float dt) {
		if (!_reaction.tick(dt))
			return;
		
		float range = getWeaponRange(_object);
		range *= range;
		//LOG_DEBUG(("range = %g", range));

		std::set<std::string> targets;

		if (_aim_missiles)
			targets.insert("missile");
	
		targets.insert("player");
		targets.insert("trooper");
		targets.insert("kamikaze");
	
		v3<float> pos, vel;
		if (getNearest(targets, pos, vel) && pos.quick_length() <= range) {
			_state.fire = true;
			_direction = pos;
			_direction.normalize();
			setDirection(_direction.getDirection(getDirectionsNumber()) - 1);
			
		} else _state.fire = false;
	}
private: 
	Alarm _reaction; 
};


void Trooper::tick(const float dt) {
	setDirection(_velocity.getDirection8() - 1);
	Object::tick(dt);
	
	const std::string state = getState();
	if (_velocity.is0()) {
		if (state != "hold" && state != "fire") {
			cancelAll();
			play("hold", true);
		}
	} else {
		if (state == "hold") {
			cancelAll();
			play("run", true);
		}		
	}
	
	if (_fire.tick(dt) && _state.fire) {
		_fire.reset();
		if (getState() != "fire")
			playNow("fire");
		spawn(_object, _object, v3<float>::empty, _direction);
	}
}

void Trooper::onSpawn() {
	if (_object == "thrower-missile") {
		GET_CONFIG_VALUE("objects.thrower.fire-rate", float, fr, 3);
		_fire.set(fr);
	} else if (_object == "machinegunner-bullet") {
		GET_CONFIG_VALUE("objects.machinegunner.fire-rate", float, fr, 0.2);
		_fire.set(fr);
	} else throw_ex(("unsupported weapon %s", _object.c_str()));
	
	play("hold", true);
}

void Trooper::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
		spawn("corpse", "dead-machinegunner", v3<float>::empty, v3<float>::empty);
	} else if (event == "collision" && emitter != NULL && emitter->classname == "vehicle") {
		if (_velocity.same_sign(getRelativePosition(emitter)) &&
			World->attachVehicle(this, emitter))
			return;
	}
	Object::emit(event, emitter);
}


Object* Trooper::clone() const  {
	return new Trooper(*this);
}


REGISTER_OBJECT("machinegunner-player", Trooper, ("player", "machinegunner-bullet", true));
REGISTER_OBJECT("machinegunner-in-watchtower", TrooperInWatchTower, ("machinegunner-bullet", true));
REGISTER_OBJECT("thrower-in-watchtower", TrooperInWatchTower, ("thrower-missile", false));
