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
#include "mrt/random.h"
#include "special_owners.h"

class AITrooper : public Trooper, ai::Herd {
public:
	AITrooper(const std::string &object, const bool aim_missiles) : 
		Trooper("trooper", object), _reaction(true), _target_dir(-1) {
			if (aim_missiles)
				_targets.insert("missile");
	
			_targets.insert("fighting-vehicle");
			_targets.insert("trooper");
			_targets.insert("kamikaze");
			_targets.insert("boat");
			_targets.insert("helicopter");
			_targets.insert("monster");
	}
	virtual void onSpawn();
	virtual void serialize(mrt::Serializator &s) const {
		Trooper::serialize(s);
		s.add(_reaction);
		s.add(_target_dir);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Trooper::deserialize(s);
		s.get(_reaction);
		s.get(_target_dir);
	}
	virtual void calculate(const float dt);
	virtual Object* clone() const;

	virtual void onIdle(const float dt);
	
private: 
	virtual const int getComfortDistance(const Object *other) const;

	Alarm _reaction;
	int _target_dir;
	
	//no need for serialize it:
	std::set<std::string> _targets;
};

const int AITrooper::getComfortDistance(const Object *other) const {
	GET_CONFIG_VALUE("objects.ai-trooper.comfort-distance", int, cd, 80);
	return (other == NULL || other->classname == "trooper")?cd:-1;
}

void AITrooper::onIdle(const float dt) {
	int summoner = getSummoner();
	if (summoner != 0 && summoner != OWNER_MAP) {
		float range = getWeaponRange(_object);
		ai::Herd::calculateV(_velocity, this, summoner, range);
	} else _velocity.clear();
	_state.fire = false;

	GET_CONFIG_VALUE("objects.ai-trooper.rotation-time", float, rt, 0.05);
	limitRotation(dt, rt, true, false);
	updateStateFromVelocity();	
}

void AITrooper::onSpawn() {
	GET_CONFIG_VALUE("objects.ai-trooper.reaction-time", float, rt, 0.3);
	mrt::randomize(rt, rt / 10);
	//LOG_DEBUG(("rt = %g", rt));
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
	
	_state.fire = false;
	
	v2<float> vel;
	
	int range = (int)getWeaponRange(_object);
	int trs, tra;
	
	Config->get("objects." + registered_name + ".targeting-range(stable)", trs, range * 2 / 3);
	Config->get("objects." + registered_name + ".targeting-range(alerted)", tra, range);
	int tt = (hp < max_hp)?tra:trs;

	_target_dir = getTargetPosition(_velocity, _targets, (float)tt);
	if (_target_dir >= 0) {
		//LOG_DEBUG(("target: %g %g %g", tp.x, tp.y, tp.length()));
		/*
		Way way;
		if (findPath(tp, way)) {
		setWay(way);
			calculateWayVelocity();
		}
		*/
		if (_velocity.length() >= 9) {
			quantizeVelocity();
			_direction.fromDirection(getDirection(), getDirectionsNumber());
		} else {
			_velocity.clear();
			setDirection(_target_dir);
			//LOG_DEBUG(("%d", _target_dir));
			_direction.fromDirection(_target_dir, getDirectionsNumber());
			_state.fire = true;
		}
	
	} else {
		_velocity.clear();
		_target_dir = -1;
		onIdle(dt);
	}
}
//==============================================================================
class TrooperInWatchTower : public Trooper {
public: 
	TrooperInWatchTower(const std::string &object, const bool aim_missiles) : 
		Trooper("trooper", object), _reaction(true) {
			if (aim_missiles)
				_targets.insert("missile");
	
			_targets.insert("fighting-vehicle");
			_targets.insert("monster");
			_targets.insert("trooper");
			_targets.insert("kamikaze");
			_targets.insert("boat");		
			_targets.insert("helicopter");
	}
	virtual Object * clone() const { return new TrooperInWatchTower(*this); }
	
	virtual void onSpawn() { 
		GET_CONFIG_VALUE("objects.trooper.reaction-time", float, rt, 0.1);
		mrt::randomize(rt, rt/10);
		_reaction.set(rt);
	
		Trooper::onSpawn();
	}

	virtual void serialize(mrt::Serializator &s) const {
		Trooper::serialize(s);
		s.add(_reaction);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Trooper::deserialize(s);
		s.get(_reaction);
	}
	
	virtual void calculate(const float dt) {
		if (!_reaction.tick(dt))
			return;
		
		float range = getWeaponRange(_object);
		//LOG_DEBUG(("range = %g", range));

		_state.fire = false;

		const Object * result = NULL;
		float dist = -1;
		
		std::set<const Object *> objects;
		enumerateObjects(objects, range, &_targets);
		for(std::set<const Object *>::const_iterator i = objects.begin(); i != objects.end(); ++i) {
			const Object *target = *i;
			if (hasSameOwner(target) || target->aiDisabled())
				continue;
			
			v2<float> dpos = getRelativePosition(target);
			if (checkDistance(getCenterPosition(), target->getCenterPosition(), getZ(), true)) {
				if (result == NULL || dpos.quick_length() < dist) {
					result = target;
					dist = dpos.quick_length();
				}
			}
		}
		
		if (result != NULL) {
			_state.fire = true;
			_direction = getRelativePosition(result);
			_direction.normalize();
			setDirection(_direction.getDirection(getDirectionsNumber()) - 1);
		}
	}
private: 
	Alarm _reaction; 

	//no need to serialize it
	std::set<std::string> _targets;
};

REGISTER_OBJECT("machinegunner", AITrooper, ("machinegunner-bullet", true));
REGISTER_OBJECT("thrower", AITrooper, ("thrower-missile", false));

REGISTER_OBJECT("machinegunner-in-watchtower", TrooperInWatchTower, ("machinegunner-bullet", true));
REGISTER_OBJECT("thrower-in-watchtower", TrooperInWatchTower, ("thrower-missile", false));
