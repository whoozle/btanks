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

#include "trooper.h"
#include "ai/herd.h"
#include "config.h"
#include "resource_manager.h"

class AITrooper : public Trooper, ai::Herd {
public:
	AITrooper(const std::string &object, const bool aim_missiles) : Trooper("trooper", object, aim_missiles), _reaction(true) {}
	virtual void onSpawn();
	virtual void serialize(mrt::Serializator &s) const {
		Trooper::serialize(s);
		_reaction.serialize(s);
		_target.serialize(s);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Trooper::deserialize(s);
		_reaction.deserialize(s);
		_target.deserialize(s);
	}
	virtual void calculate(const float dt);
	virtual Object* clone() const;

	virtual void onIdle(const float dt);
	
private: 
	virtual const int getComfortDistance(const Object *other) const;

	Alarm _reaction;
	v3<float> _target;
};

const int AITrooper::getComfortDistance(const Object *other) const {
	GET_CONFIG_VALUE("objects.ai-trooper.comfort-distance", int, cd, 80);
	return (other == NULL || other->classname == "trooper")?cd:-1;
}

void AITrooper::onIdle(const float dt) {
	int summoner = getSummoner();
	if (summoner != 0 && summoner != -42) {
		float range = getWeaponRange(_object);
		ai::Herd::calculateV(_velocity, this, summoner, range);
	} else _velocity.clear();
	_state.fire = false;

	GET_CONFIG_VALUE("objects.trooper.rotation-time", float, rt, 0.05);
	limitRotation(dt, rt, true, false);
}

void AITrooper::onSpawn() {
	GET_CONFIG_VALUE("objects.trooper.reaction-time", float, rt, 0.1);
	_reaction.set(rt);
	
	Trooper::onSpawn();
}

Object* AITrooper::clone() const  {
	return new AITrooper(*this);
}


void AITrooper::calculate(const float dt) {
	//calculateWayVelocity();
	//LOG_DEBUG(("calculate"));
	if (!_reaction.tick(dt))
		return;
	if (getState() == "fire") {
		_state.fire = true; //just to be sure.
		return;
	}
	
	std::set<std::string> targets;

	if (_aim_missiles)
		targets.insert("missile");
	
	targets.insert("player");
	targets.insert("trooper");
	targets.insert("kamikaze");
	targets.insert("boat");
	
	v3<float> vel;
	if (getNearest(targets, _target, vel)) {
		v3<float> tp;
		float r = getWeaponRange(_object);
		if (_target.quick_length() > r * r) {
			onIdle(dt);
			return;
		} else {
			if (getTargetPosition(tp, _target, _object)) {
				//LOG_DEBUG(("target: %g %g %g", tp.x, tp.y, tp.length()));
				/*
				Way way;
				if (findPath(tp, way)) {
					setWay(way);
					calculateWayVelocity();
				}
				*/
				_velocity = tp;
				quantizeVelocity();
				_direction.fromDirection(getDirection(), getDirectionsNumber());
		
				if (tp.length() < 16)
					_velocity.clear();
			} else onIdle(dt);
		}	
	}
	
	_state.fire = _velocity.is0() && !_target.is0();
	if (_state.fire) {
		_direction = _target;
		_direction.quantize8();
		setDirection(_direction.getDirection8() - 1 );
	}
}

REGISTER_OBJECT("machinegunner", AITrooper, ("machinegunner-bullet", true));
REGISTER_OBJECT("thrower", AITrooper, ("thrower-missile", false));
