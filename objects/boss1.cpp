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
#include "game_monitor.h"
#include "config.h"
#include "object.h"
#include "ai/waypoints.h"
#include "alarm.h"
#include "world.h"

class Boss1 : public Object, public ai::Waypoints {
public: 
	Boss1();

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
		s.add(_left);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		//ai::Waypoints::deserialize(s);
		s.get(_reaction);
		s.get(_fire);
		s.get(_alt_fire);
		s.get(_left);
	}
private: 
	const int getTargetPosition2(const std::set<std::string> &targets, const std::string &weapon) const;


	Alarm _reaction;
	Alarm _fire, _alt_fire;
	bool _left;
	
	std::set<std::string> _enemies;
};

const int Boss1::getTargetPosition2(const std::set<std::string> &targets, const std::string &weapon) const {
	if (GameMonitor->disabled(this))
		return -1;

	float range = getWeaponRange(weapon);
	
	const int dirs = getDirectionsNumber();
	
	std::set<const Object *> objects;
	World->enumerateObjects(objects, this, range, &targets);
	for(std::set<const Object *>::const_iterator i = objects.begin(); i != objects.end(); ++i) {
		const Object *o = *i;
		if (hasSameOwner(o))
			continue;
		
		v2<float> tp = getRelativePosition(o);
		float dist = tp.length();
		
		for(int d = 0; d < dirs; ++d) {
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

void Boss1::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
		spawn("corpse", "dead-" + animation, v2<float>::empty, v2<float>::empty);
	} 
	Object::emit(event, emitter);
}

void Boss1::onSpawn() {
	_enemies.insert("player");
	_enemies.insert("trooper");
	
	ai::Waypoints::onSpawn(this);
	play("hold", true);

	float rt;
	Config->get("objects." + registered_name + ".reaction-time", rt, 0.1f);
	_reaction.set(rt);

	Config->get("objects." + registered_name + ".fire-rate", rt, 0.1f);
	_fire.set(rt);
	//Config->get("objects." + registered_name + ".alt-fire-rate", rt, 1.0f);
	_alt_fire.set(rt);
}

Boss1::Boss1() : Object("monster"), _reaction(true), _fire(false), _alt_fire(false), _left(false) {}

void Boss1::calculate(const float dt) {
	if (_reaction.tick(dt)) {
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
			spawn("helicopter-bullet", _left?"helicopter-bullet-left":"helicopter-bullet-right", v2<float>::empty, _direction);
			_left = !_left;
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


REGISTER_OBJECT("uberzombie", Boss1, ());
