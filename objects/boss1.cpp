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
#include "config.h"
#include "object.h"
#include "ai/waypoints.h"
#include "alarm.h"
#include "world.h"

class Boss1 : public Object, public ai::Waypoints {
public: 
	Boss1(const float fire_shift);

	virtual void addDamage(Object *from, const int hp, const bool emitDeath) {
		_stable.reset();
		Object::addDamage(from, hp, emitDeath);
	}
	
	virtual void calculate(const float dt);
	virtual void tick(const float dt);
	virtual void onSpawn();
	virtual Object * clone() const;
	
	void emit(const std::string &event, Object * emitter);
	
	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		//ai::Waypoints::serialize(s);
		s.add(_reaction);
		s.add(_fire);
		s.add(_alt_fire);
		s.add(_stable);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		//ai::Waypoints::deserialize(s);
		s.get(_reaction);
		s.get(_fire);
		s.get(_alt_fire);
		s.get(_stable);
	}
protected: 
	void addFireShift(v2<float>& pos, const int d) const {
		const int dirs = getDirectionsNumber();
		const int shift_d = (5 + d) % dirs;
		v2<float> fire_shift;
		fire_shift.fromDirection(shift_d, dirs);
		fire_shift *= _fire_shift;
		pos += fire_shift;
	}

private: 
	const int getTargetPosition2(const std::set<std::string> &targets, const std::string &weapon) const;


	Alarm _reaction;
	Alarm _fire, _alt_fire;
	Alarm _stable;
	
	std::set<std::string> _enemies;
	float _fire_shift; //do not serialize
};

const int Boss1::getTargetPosition2(const std::set<std::string> &targets, const std::string &weapon) const {
	float range = getWeaponRange(weapon);
	
	const int dirs = getDirectionsNumber();
	
	std::set<const Object *> objects;
	World->enumerateObjects(objects, this, range, &targets);
	for(std::set<const Object *>::const_iterator i = objects.begin(); i != objects.end(); ++i) {
		const Object *o = *i;
		if (hasSameOwner(o))
			continue;
		
		v2<float> cp, my_cp;
		getCenterPosition(my_cp);
		o->getCenterPosition(cp);
		
		
		for(int d = 0; d < dirs; ++d) {
			v2<float> firepoint = my_cp;
			addFireShift(firepoint, d);

			v2<float> tp = cp - firepoint;
			float dist = tp.length();
			
			v2<float> dir;
			dir.fromDirection(d, dirs);
			dir *= dist;
			
			dir -= tp;
			float l = dir.length();
			if (l < 25) 
				return d;
			//LOG_DEBUG(("%d: %g", d, l));
		}
	}	
	return -1;
}

void Boss1::onSpawn() {
	_enemies.insert("player");
	_enemies.insert("trooper");
	
	ai::Waypoints::onSpawn(this);
	play("hold", true);

	float rt;
	Config->get("objects." + registered_name + ".reaction-time", rt, 0.05f);
	_reaction.set(rt);

	Config->get("objects." + registered_name + ".fire-rate", rt, 0.1f);
	_fire.set(rt);
	//Config->get("objects." + registered_name + ".alt-fire-rate", rt, 1.0f);
	_alt_fire.set(rt);
}


void Boss1::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
		spawn("escaping-" + registered_name, "escaping-" + animation, v2<float>(), v2<float>());
	} 
	Object::emit(event, emitter);
}

Boss1::Boss1(const float fire_shift) : 
	Object("monster"), _reaction(true), _fire(false), _alt_fire(false), _stable(1.0f, false), _fire_shift(fire_shift) {}

void Boss1::calculate(const float dt) {
	bool stable = _stable.tick(dt);

	if (aiDisabled())
		return;

	if (!stable)
		_state.fire = false;
	
	if (_reaction.tick(dt) && stable) {
		_state.fire = false;
		int dir = getTargetPosition2(_enemies, "helicopter-bullet");
		if (dir >= 0) {
			//LOG_DEBUG(("target dir = %d", dir));
			_state.fire = true;
			setDirection(dir);
			_direction.fromDirection(dir, getDirectionsNumber());
		}
	}
	
	if (_state.fire) {
		_velocity.clear();
		updateStateFromVelocity();
		return;	
	}

	ai::Waypoints::calculate(this, dt);

	float rt;
	Config->get("objects." + registered_name + ".rotation-time", rt, 0.05f);
	limitRotation(dt, rt, true, false);
	updateStateFromVelocity();
}


void Boss1::tick(const float dt) {
	Object::tick(dt);
	if (_state.fire) { 
		if (getState() != "fire") {
			cancelAll();
			play("fire", true);
		}
		if ( _fire.tick(dt)) {
			_fire.reset();
			v2<float> dpos;
			//addFireShift(dpos, getDirection());
			spawn("helicopter-bullet", "uberzombie-bullet", dpos, _direction);
		}
	} else if (_velocity.is0() && getState() != "hold") {
		cancelAll();
		play("hold", true);
	} else if (!_velocity.is0() && getState() != "walk") {
		cancelAll();
		play("walk", true);
	}
}


Object * Boss1::clone() const {
	return new Boss1(*this);
}

class Boss1Escaping : public Object {
public:
	Boss1Escaping() : Object("boss") { hp = -1; }
	Object * clone() const { return new Boss1Escaping(*this); }
	void onSpawn() {
		play("prepare", false);
		play("escape", true);
	}
	void calculate(const float dt) {
		if (getState() == "escape") {
			_velocity.x = -1;
			_velocity.y = -3;
		}
	}
};


REGISTER_OBJECT("uberzombie", Boss1, (40));
REGISTER_OBJECT("escaping-uberzombie", Boss1Escaping, ());
