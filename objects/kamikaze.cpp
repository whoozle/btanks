
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

class Kamikaze : public Object {
public:
	Kamikaze() : 
		Object("kamikaze"), _reaction(true) {}
	
	virtual void tick(const float dt);
	virtual void calculate(const float dt);

	virtual Object * clone() const;
	virtual void onSpawn();
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);

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

void Kamikaze::calculate(const float dt) {
	calculateWayVelocity();
	if (!_reaction.tick(dt))
		return;
	
	v3<float> vel;
	static std::vector<std::string> targets;
	if (targets.empty()) {
		targets.push_back("train");
		targets.push_back("player");
	}

	if (getNearest(targets, _velocity, vel)) {
		_velocity.quantize8();
		setDirection(_velocity.getDirection8() - 1);
	} else _velocity.clear();
}

void Kamikaze::tick(const float dt) {
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
	Object::tick(dt);
}

void Kamikaze::onSpawn() {
	GET_CONFIG_VALUE("objects.kamikaze.reaction-time", float, rt, 0.1);
	_reaction.set(rt);
	play("hold", true);
}

void Kamikaze::emit(const std::string &event, BaseObject * emitter) {
	if (event == "death") {
		spawn("explosion", "missile-explosion");
		Object::emit(event, emitter);
	} else if (event == "collision") {
		if (emitter->classname == "missile" || emitter->classname == "bullet") {
			Object::emit(event, emitter);
			return;
		}
		
		if (emitter) 
			emitter->addDamage(this, hp);
		emit("death", emitter);
	} else 
		Object::emit(event, emitter);
}


Object* Kamikaze::clone() const  {
	Object *a = new Kamikaze(*this);
	return a;
}

REGISTER_OBJECT("kamikaze", Kamikaze, ());
