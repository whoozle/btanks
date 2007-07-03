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

class Machinegunner : public Object {
public:
	Machinegunner(const std::string &classname) : Object(classname), _fire(true) { impassability = 0; setDirectionsNumber(16); }
	virtual Object * clone() const { return new Machinegunner(*this); }
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
	Alarm _fire;
};

void Machinegunner::onSpawn() {
	play("main", true);

	GET_CONFIG_VALUE("objects.machinegunner.fire-rate", float, fr, 0.2);
	_fire.set(fr);
}

void Machinegunner::tick(const float dt) {
	if (_fire.tick(dt) && _state.fire) {
		spawn("machinegunner-bullet", "vehicle-machinegunner-bullet", v2<float>(), _direction);
	}
}

void Machinegunner::calculate(const float dt) {
	static std::set<std::string> targets;
	if (targets.empty()) {
		targets.insert("missile");
		targets.insert("fighting-vehicle");
		targets.insert("trooper");
		targets.insert("kamikaze");
		targets.insert("boat");
		targets.insert("helicopter");
		targets.insert("monster");
	}
	
	v2<float> pos, vel;
	

	GET_CONFIG_VALUE("objects.machinegunner-on-launcher.targeting-range", int, range, (int)getWeaponRange("machinegunner-bullet"));

	
	if (!getNearest(targets, range, pos, vel, true)) {
		_state.fire = false;
		Object::calculate(dt);
		return;
	}
	_direction = pos;
	_state.fire = true;
	_direction.quantize16();
	setDirection(_direction.getDirection16() - 1);
	//LOG_DEBUG(("found! %g %g dir= %d", _direction.x, _direction.y, dir));
}

void Machinegunner::emit(const std::string &event, Object * emitter) {
	if (event == "hold" || event == "move" || event == "launch")
		return;
	Object::emit(event, emitter);
}
const bool Machinegunner::take(const BaseObject *obj, const std::string &type) {
	return false;
}

void Machinegunner::serialize(mrt::Serializator &s) const {
	Object::serialize(s);
	_fire.serialize(s);
}

void Machinegunner::deserialize(const mrt::Serializator &s) {
	Object::deserialize(s);
	_fire.deserialize(s);
}


REGISTER_OBJECT("machinegunner-on-launcher", Machinegunner, ("machinegunner-on-launcher"));
