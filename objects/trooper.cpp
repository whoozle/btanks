
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
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		_reaction.deserialize(s);
		s.get(_object);
		s.get(_aim_missiles);
		_fire.deserialize(s);
	}	

private: 
	Alarm _reaction;
	std::string _object;
	bool _aim_missiles;
	Alarm _fire;
};

void Trooper::calculate(const float dt) {
	calculateWayVelocity();
	if (!_reaction.tick(dt))
		return;
	
	std::vector<std::string> targets;

	targets.push_back("missile");
	targets.push_back("player");
	targets.push_back("trooper");
	
	v3<float> pos, vel;
	if (getNearest(targets, pos, vel)) {
		v3<float> tp;
		getTargetPosition(tp, pos, _object, 16);
		LOG_DEBUG(("target: %g %g", tp.x, tp.y));
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
		if (tp.length() < 8)
			_velocity.clear();
	}
	_state.fire = _velocity.is0();
}

void Trooper::tick(const float dt) {
	Object::tick(dt);
	
	const std::string state = getState();
	if (_velocity.is0()) {
		if (state != "hold") {
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
		playNow("fire");
		spawn(_object, _object, v3<float>::empty, _direction);
	}
}

void Trooper::onSpawn() {
	GET_CONFIG_VALUE("objects.trooper.reaction-time", float, rt, 0.1);
	_reaction.set(rt);

	GET_CONFIG_VALUE("objects.machinegunner.fire-rate", float, fr, 0.2);
	_fire.set(fr);
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
