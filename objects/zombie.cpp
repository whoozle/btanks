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
#include "ai/herd.h"

class BaseZombie : public Object {
public: 
	virtual Object * clone() const { return new BaseZombie(*this); }
	BaseZombie(const std::string &classname): Object(classname), _can_punch(true) {}

	virtual void onSpawn();
	virtual void tick(const float dt);
	virtual void emit(const std::string &event, Object * emitter = NULL);

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_can_punch);
	}

	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_can_punch);
	}	

private:  
	bool _can_punch;	
};

void BaseZombie::onSpawn() {
	play("hold", true);
	disown();
}

void BaseZombie::tick(const float dt) {
	Object::tick(dt);

	if (_state.fire && getState() != "punch") {
		_can_punch = true;
		playNow("punch");
		return;
	}

	if (_velocity.is0()) {
		if (getState() != "hold") {
			cancelAll();
			play("hold", true);
		}
	} else {
		if (getState() == "hold") {
			cancelAll();
			play("walk", true);
		}		
	}
}


void BaseZombie::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
		spawn("corpse(zombie-death)", "dead-zombie", v2<float>(), v2<float>());
	} else if (emitter != NULL && event == "collision") {
		if (getState() != "punch" && emitter->registered_name != "zombie") {
			_state.fire = true;
		}	
		if (_state.fire && _can_punch && getStateProgress() >= 0.5 && getState() == "punch" && emitter->registered_name != "zombie") {
			_can_punch = false;
			
			GET_CONFIG_VALUE("objects.zombie.damage", int, kd, 15);
		
			if (emitter) 
				emitter->addDamage(this, kd);
			
			return;
		}

	}
	Object::emit(event, emitter);
}




/*============================================
				ai zombie
============================================*/

class Zombie : public BaseZombie, public ai::Herd{
public:
	Zombie(const std::string &classname) : 
	BaseZombie(classname), _reaction(true) {
		_targets.insert("fighting-vehicle");
		_targets.insert("trooper");
		_targets.insert("watchtower");
		_targets.insert("creature");
		_targets.insert("civilian");
	}
	
	virtual void calculate(const float dt);

	virtual Object * clone() const;
	virtual void onSpawn();

	virtual void serialize(mrt::Serializator &s) const {
		BaseZombie::serialize(s);
		s.add(_reaction);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		BaseZombie::deserialize(s);
		s.get(_reaction);
	}	
	
	virtual void onIdle(const float dt);

	const int getComfortDistance(const Object *other) const;

private: 
	Alarm _reaction;
	//no need for serialization (ONLY IF TARGETS INIT'ED IN CTOR AND DOESNT MODIFIED ANYWHERE
	std::set<std::string> _targets;
};

const int Zombie::getComfortDistance(const Object *other) const {
	GET_CONFIG_VALUE("objects.zombie.comfort-distance", int, cd, 120);
	return (other == NULL || other->registered_name == registered_name)?cd:-1; //fixme names if you want
}


void Zombie::onIdle(const float dt) {
	_state.fire = false;
	
	GET_CONFIG_VALUE("objects.zombie.targeting-range(stable)", int, trs, 600);
	GET_CONFIG_VALUE("objects.zombie.targeting-range(alerted)", int, tra, 900);
	int tt = (hp < max_hp)?tra:trs;

	ai::Herd::calculateV(_velocity, this, 0, tt);
}


void Zombie::calculate(const float dt) {
	if (!_reaction.tick(dt))
		return;
	
	v2<float> vel;
	
	GET_CONFIG_VALUE("objects.zombie.targeting-range(stable)", int, trs, 600);
	GET_CONFIG_VALUE("objects.zombie.targeting-range(alerted)", int, tra, 900);
	int tt = (hp < max_hp)?tra:trs;
	
	if (getNearest(_targets, tt, _velocity, vel)) {
		if (_velocity.quick_length() > size.quick_length())
			_state.fire = false;
		
		_velocity.normalize();
		quantizeVelocity();		
	} else {
		_state.fire = false;
		if (!_variants.has("no-herd"))
			onIdle(dt);
	}

	GET_CONFIG_VALUE("objects.zombie.rotation-time", float, rt, 0.1);
	limitRotation(dt, rt, true, false);
}

void Zombie::onSpawn() {
	BaseZombie::onSpawn();

	float rt, drt = 0.5;
	
	Config->get("objects." + registered_name + ".reaction-time", rt, drt);
	mrt::randomize(rt, rt/10);
	_reaction.set(rt);
}

Object* Zombie::clone() const  {
	return new Zombie(*this);
}

REGISTER_OBJECT("zombie", Zombie, ("monster"));
REGISTER_OBJECT("zombie-player", BaseZombie, ("monster"));
