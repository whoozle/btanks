
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

class Heli : public Object {
public:
	Heli() : 
		Object("helicopter"), _reaction(true), _fire(false), _left(false) {
		impassability = -1;	
	}
	
	virtual void tick(const float dt);
	virtual void calculate(const float dt);

	virtual Object * clone() const;
	virtual void onSpawn();
	virtual void emit(const std::string &event, Object * emitter = NULL);

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		_reaction.serialize(s);
		s.add(_left);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		_reaction.deserialize(s);
		s.get(_left);
	}	

private: 
	Alarm _reaction, _fire;
	bool _left;
};

void Heli::calculate(const float dt) {
	if (!_reaction.tick(dt))
		return;
		
	_state.fire  = true;

	GET_CONFIG_VALUE("objects.helicopter.rotation-time", float, rt, 0.2);
	limitRotation(dt, rt, true, false);	
}

void Heli::tick(const float dt) {
	Object::tick(dt);
	
	if (_state.fire && _fire.tick(dt)) {
		_fire.reset();
		spawn("helicopter-bullet", _left?"helicopter-bullet-left":"helicopter-bullet-right", v2<float>::empty, _direction);
		_left = !_left;
	}
}

void Heli::onSpawn() {
	GET_CONFIG_VALUE("objects.helicopter.reaction-time", float, rt, 0.1);
	mrt::randomize(rt, rt/10);
	_reaction.set(rt);

	GET_CONFIG_VALUE("objects.helicopter.fire-rate", float, fr, 0.1);
	_fire.set(fr);

	play("move", true);
}

void Heli::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
		spawn("explosion", "nuclear-explosion");
	} else if (event == "collision") {
	}
	
	Object::emit(event, emitter);
}


Object* Heli::clone() const  {
	return new Heli(*this);
}

REGISTER_OBJECT("helicopter", Heli, ());
