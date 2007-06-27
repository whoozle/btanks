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
#include "config.h"
#include "resource_manager.h"

class Cannon : public Object {
public:
	Cannon(const int dir) : Object("trooper"), _fire(false), _reaction(true) {
		setDirection(dir);
	}
	virtual Object* clone() const  { return new Cannon(*this); }
	
	virtual void calculate(const float dt);
	virtual void tick(const float dt);
	virtual void emit(const std::string &event, Object * emitter);

	virtual void onSpawn();

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		_fire.serialize(s);
		_reaction.serialize(s);
	}

	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		_fire.deserialize(s);
		_reaction.deserialize(s);
	}

private:
	Alarm _fire, _reaction;
};

void Cannon::calculate(const float dt) {
	if (!_reaction.tick(dt))
		return;
	
	static std::set<std::string> targets;
	if (targets.empty()) {
		targets.insert("fighting-vehicle");
		targets.insert("trooper");
		targets.insert("monster");
		targets.insert("kamikaze");
	}
	static float range = getWeaponRange("cannon-bullet");
	v2<float> pos, vel;
	if (getNearest(targets, range, pos, vel)) {
		pos.normalize();
		setDirection(pos.getDirection(getDirectionsNumber()) - 1);
		_direction = pos;
		_state.fire = true;
	} else _state.fire = false;
}

void Cannon::tick(const float dt) {
	Object::tick(dt);
	if (getState() == "real-fire") {
		cancel();
		spawn("cannon-bullet", "cannon-bullet", v2<float>(), _direction);
	}
	
	bool can_fire = _fire.tick(dt);
	if (_state.fire && can_fire) {
		_fire.reset();
		
		if (getState() == "hold") {
			cancelAll();
			play("fire", false);
			play("real-fire", true);
			play("after-fire", false);
			play("hold", true);
		}
	}
}

void Cannon::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
		spawn("corpse", "dead-" + registered_name);
	}
	Object::emit(event, emitter);
}


void Cannon::onSpawn() {
	GET_CONFIG_VALUE("objects.cannon.fire-rate", float, fr, 2);
	_fire.set(fr);
	GET_CONFIG_VALUE("objects.cannon.reaction-time", float, rt, 0.105);
	_reaction.set(rt);	
	play("hold", true);
}

REGISTER_OBJECT("cannon", Cannon, (6));
