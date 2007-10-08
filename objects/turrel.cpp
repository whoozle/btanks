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
#include "alarm.h"
#include "resource_manager.h"
#include "config.h"
#include "mrt/random.h"

class Turrel : public Object {
public:
	Turrel(const std::string &classname) : 
		Object(classname), _reaction(true), _fire(true), _left(false) { impassability = 1; setDirectionsNumber(8); }
	
	virtual Object * clone() const { return new Turrel(*this); }
	virtual void onSpawn();
	virtual void tick(const float dt);
	virtual void calculate(const float dt);
	virtual void emit(const std::string &event, Object * emitter = NULL);
	virtual const bool take(const BaseObject *obj, const std::string &type);
	virtual const std::string getType() const { return "machinegunner"; }
	virtual const int getCount() const { return -1; }

	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s); 

private: 
	Alarm _reaction, _fire;
	bool _left;
};

void Turrel::onSpawn() {
	play("hold", true);

	float fr;
	Config->get("objects." + registered_name + ".fire-rate", fr, 0.1);
	_fire.set(fr);

	GET_CONFIG_VALUE("objects.turrel.reaction-time", float, rt, 0.2);
	mrt::randomize(rt, rt / 10);
	_reaction.set(rt);
}

void Turrel::tick(const float dt) {
	Object::tick(dt);
	if (_fire.tick(dt) && _state.fire) {
		bool air_mode = (_parent != NULL)?_parent->getPlayerState().alt_fire:true;
		cancelAll();
		play(_left? "fire-left": "fire-right", false);
		play("hold", true);
		std::string animation = mrt::formatString("buggy-%s-%s", air_mode?"air-bullet":"bullet", _left?"left":"right");
		Object *bullet = spawn("buggy-bullet", animation, v2<float>(), _direction);
		
		bullet->setZ(air_mode? bullet->getZ() + 2000:getZ() - 1, true);
		_left = !_left;
	}
}

void Turrel::calculate(const float dt) {
	if (!_reaction.tick(dt))
		return;

	static std::set<std::string> targets;
	if (targets.empty()) {
		targets.insert("missile");
		targets.insert("fighting-vehicle");
		targets.insert("trooper");
		targets.insert("kamikaze");
		targets.insert("boat");
		targets.insert("helicopter");
		targets.insert("monster");
		targets.insert("watchtower");
	}
	
	bool air_mode = (_parent != NULL)?_parent->getPlayerState().alt_fire:true;
	if (air_mode) {
		v2<float> pos, vel;
		int z0 = getZ();
		setZ(z0 + 2000, true); //temporary move up turrel %) hack for air mode :)
		if (getNearest(targets, getWeaponRange("helicopter-bullet"), pos, vel, true)) {
			_direction = pos;
			_state.fire = true;
			_direction.quantize8();
			setDirection(_direction.getDirection8() - 1);
		} else {
			_state.fire = false;
		}
		setZ(z0, true);
	} else {
		if (_parent != NULL) {
			_state.fire = _parent->getPlayerState().fire;

			int idx = _parent->getDirection();
			setDirection(idx);
			_direction.fromDirection(idx, getDirectionsNumber());
		}
	}
}

void Turrel::emit(const std::string &event, Object * emitter) {
	if (event == "hold" || event == "move") {
		cancelAll();
		play(event, true);
		return;
	}
	Object::emit(event, emitter);
}

const bool Turrel::take(const BaseObject *obj, const std::string &type) {
	return false;
}

void Turrel::serialize(mrt::Serializator &s) const {
	Object::serialize(s);
	s.add(_fire);
	s.add(_left);	
}

void Turrel::deserialize(const mrt::Serializator &s) {
	Object::deserialize(s);
	s.get(_fire);
	s.get(_left);	
}


REGISTER_OBJECT("turrel", Turrel, ("turrel"));
