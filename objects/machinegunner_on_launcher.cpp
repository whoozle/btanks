/* Battle Tanks Game
 * Copyright (C) 2006-2009 Battle Tanks team
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

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
*/

#include "object.h"
#include "alarm.h"
#include "registrar.h"
#include "config.h"
#include "ai/targets.h"

class Machinegunner : public Object {
public:
	Machinegunner(const char *object) : 
		Object("trooper-on-launcher"), _fire(true), _object(object)
		{ hp = -1; impassability = 0; set_directions_number(16); }

	virtual Object * clone() const { return new Machinegunner(*this); }
	virtual void on_spawn();
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
	std::string _object;
};

void Machinegunner::on_spawn() {
	play("main", true);

	float fr;
	Config->get("objects.trooper-on-launcher-with-" + _object + ".fire-rate", fr, 0.2);
	_fire.set(fr);
}

void Machinegunner::tick(const float dt) {
	Object::tick(dt);
	if (_fire.tick(dt) && _state.fire) {
		spawn(_object, _object, v2<float>(), _direction);
	}
}

void Machinegunner::calculate(const float dt) {
	if (_parent != NULL) {
		if (_parent->classname != "fighting-vehicle") {
			_state.fire = _state.alt_fire = false;
			return;
		}
	}

	v2<float> pos, vel;

	GET_CONFIG_VALUE("objects.machinegunner-on-launcher.targeting-range", int, range, (int)getWeaponRange("machinegunner-bullet"));

	if (!get_nearest(ai::Targets->troops, range, pos, vel, true)) {
		_state.fire = false;
		Object::calculate(dt);
		return;
	}
	_direction = pos;
	_state.fire = true;
	_direction.quantize16();
	set_direction(_direction.get_direction16() - 1);
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


REGISTER_OBJECT("machinegunner-on-launcher", Machinegunner, ("vehicle-machinegunner-bullet"));
REGISTER_OBJECT("thrower-on-launcher", Machinegunner, ("thrower-missile"));
