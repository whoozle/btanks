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
#include "mrt/random.h"

class Zombie : public Object {
public:
	Zombie() : 
		Object("monster"), _reaction(true) {}
	
	virtual void tick(const float dt);
	virtual void calculate(const float dt);

	virtual Object * clone() const;
	virtual void onSpawn();
	virtual void emit(const std::string &event, Object * emitter = NULL);

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		_reaction.serialize(s);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		_reaction.deserialize(s);
	}	

private: 
	Alarm _reaction;
};

void Zombie::calculate(const float dt) {
	if (!_reaction.tick(dt))
		return;
	
	v2<float> vel;
	static std::set<std::string> targets;
	if (targets.empty()) {
		targets.insert("player");
		targets.insert("trooper");
	}
	
	GET_CONFIG_VALUE("objects.zombie.targeting-range(stable)", int, trs, 600);
	GET_CONFIG_VALUE("objects.zombie.targeting-range(alerted)", int, tra, 900);
	int tt = (hp < max_hp)?tra:trs;

	if (getNearest(targets, tt, _velocity, vel)) {
		quantizeVelocity();
	} else _velocity.clear();
}

void Zombie::tick(const float dt) {
	const std::string state = getState();
	if (_velocity.is0()) {
		if (state != "hold") {
			cancelAll();
			play("hold", true);
		}
	} else {
		if (state == "hold") {
			cancelAll();
			play("walk", true);
		}		
	}
	Object::tick(dt);
}

void Zombie::onSpawn() {
	GET_CONFIG_VALUE("objects.zombie.reaction-time", float, rt, 0.1);
	mrt::randomize(rt, rt/10);
	_reaction.set(rt);
	play("hold", true);
}

void Zombie::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
		Object::emit(event, emitter);
	} else if (event == "collision") {
		if (emitter == NULL || (emitter->classname != "player" && emitter->classname != "trooper")) {
			Object::emit(event, emitter);
			return;
		}
		/*
		GET_CONFIG_VALUE("objects.zombie.damage", int, kd, 15);
		
		if (emitter) 
			emitter->addDamage(this, kd);		
		*/
	} else Object::emit(event, emitter);
}


Object* Zombie::clone() const  {
	return new Zombie(*this);
}

REGISTER_OBJECT("zombie", Zombie, ());
