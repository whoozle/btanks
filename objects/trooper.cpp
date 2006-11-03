
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

#include "object.h"
#include "resource_manager.h"
#include "alarm.h"
#include "config.h"

class Trooper : public Object {
public:
	Trooper(const std::string &object, const bool aim_missiles) : 
		Object("trooper"), _reaction(true), _object(object), _aim_missiles(aim_missiles), _fire(false) {}
	
	virtual void tick(const float dt);
	virtual void calculate(const float dt);

	virtual Object * clone() const;
	virtual void onSpawn();
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		_reaction.serialize(s);
		s.add(_object);
		s.add(_aim_missiles);
		_fire.serialize(s);
		_target.serialize(s);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		_reaction.deserialize(s);
		s.get(_object);
		s.get(_aim_missiles);
		_fire.deserialize(s);
		_target.deserialize(s);
	}	

private: 
	Alarm _reaction;
	std::string _object;
	bool _aim_missiles;
	Alarm _fire;
	v3<float> _target;
};

void Trooper::calculate(const float dt) {
	calculateWayVelocity();
	if (!_reaction.tick(dt))
		return;
	if (getState() == "fire") {
		_state.fire = true; //just to be sure.
		return;
	}
	
	std::vector<std::string> targets;

	if (_aim_missiles)
		targets.push_back("missile");
	
	targets.push_back("player");
	targets.push_back("trooper");
	targets.push_back("kamikaze");
	
	v3<float> vel;
	if (getNearest(targets, _target, vel)) {
		v3<float> tp;
		getTargetPosition(tp, _target, _object, 16);
		//LOG_DEBUG(("target: %g %g %g", tp.x, tp.y, tp.length()));
		/*
		Way way;
		if (findPath(tp, way)) {
			setWay(way);
			calculateWayVelocity();
		}
		*/
		_velocity = tp;
		_velocity.quantize8();
		setDirection(_velocity.getDirection8() - 1);
		_direction.fromDirection(getDirection(), 8);
		
		if (tp.length() < 16)
			_velocity.clear();
		
	}
	_state.fire = _velocity.is0();
	if (_state.fire) {
		_direction = _target;
		_direction.quantize8();
		setDirection(_direction.getDirection8() - 1 );
	}
}

void Trooper::tick(const float dt) {
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
		spawn(_object, _object, v3<float>::empty, _target);
	}
}

void Trooper::onSpawn() {
	GET_CONFIG_VALUE("objects.trooper.reaction-time", float, rt, 0.1);
	_reaction.set(rt);

	if (_object == "thrower-missile") {
		GET_CONFIG_VALUE("objects.thrower.fire-rate", float, fr, 3);
		_fire.set(fr);
	} else {
		GET_CONFIG_VALUE("objects.machinegunner.fire-rate", float, fr, 0.2);
		_fire.set(fr);
	}
	
	play("hold", true);
}

void Trooper::emit(const std::string &event, BaseObject * emitter) {
	Object::emit(event, emitter);
}


Object* Trooper::clone() const  {
	return new Trooper(*this);
}

REGISTER_OBJECT("machinegunner", Trooper, ("machinegunner-bullet", true));
REGISTER_OBJECT("thrower", Trooper, ("thrower-missile", false));
