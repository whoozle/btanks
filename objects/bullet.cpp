
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
#include "alarm.h"
#include "config.h"
#include "resource_manager.h"

class Bullet : public Object {
public:
	Bullet(const std::string &type, const int dirs) : Object("bullet"), _type(type), _dirs(dirs), _clone(true) {
		impassability = 1;
		piercing = true;
	}
	virtual void calculate(const float dt);
	virtual Object * clone() const;
	virtual void onSpawn();
	virtual void tick(const float dt);
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_type);
		s.add(_dirs);
		_clone.serialize(s);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_type);
		s.get(_dirs);
		_clone.deserialize(s);
	}

private: 
	std::string _type;
	int _dirs;
	Alarm _clone;
};

void Bullet::tick(const float dt) {
	Object::tick(dt);
	if (_type == "dispersion") {
		//LOG_DEBUG(("baaaaaaah!"));
		if (_clone.tick(dt)) {
			GET_CONFIG_VALUE("objects.dispersion-bullet.ttl-multiplier", float, ttl_m, 0.8);
			int d = (getDirection() + 1) % _dirs;
			v3<float> vel;
			vel.fromDirection(d, _dirs);
			Object * b = spawn(registered_name, animation, v3<float>::empty, vel);
			b->ttl = ttl * ttl_m;
			b->setOwner(getOwner());
			
			d = (_dirs + getDirection() - 1) % _dirs;
			vel.fromDirection(d, _dirs);
			b = spawn(registered_name, animation, v3<float>::empty, vel);
			b->ttl = ttl * ttl_m;
			b->setOwner(getOwner());
		}
	}
}


void Bullet::calculate(const float dt) {}

void Bullet::onSpawn() {
	GET_CONFIG_VALUE("objects.dispersion-bullet.clone-interval", float, ci, 0.1);
	_clone.set(ci);

	play("shot", false);
	play("move", true);
	
	int dir;
	if (_dirs == 8) {
		_velocity.quantize8();
		dir = _velocity.getDirection8();
	} else if (_dirs == 16) {
		_velocity.quantize16();
		dir = _velocity.getDirection16();	
	} else throw_ex(("bullet cannot handle %d directions", _dirs));
	
	setDirection(dir - 1);
}

void Bullet::emit(const std::string &event, BaseObject * emitter) {
	if (event == "collision" || event == "death") {
		v3<float> dpos;
		if (emitter)
			dpos = getRelativePosition(emitter) / 2;
			dpos.z = 0;
		if (_type == "regular") {
			spawn("explosion", "explosion", dpos);
		} else if (_type == "dirt") {
			spawn("dirt", "dirt", dpos);
		}
	
		Object::emit("death", emitter);
	} else Object::emit(event, emitter);
}


Object* Bullet::clone() const  {
	return new Bullet(*this);
}

REGISTER_OBJECT("bullet", Bullet, ("regular", 8));
REGISTER_OBJECT("dirt-bullet", Bullet, ("dirt", 8));
REGISTER_OBJECT("machinegunner-bullet", Bullet, ("regular", 16));

REGISTER_OBJECT("dispersion-bullet", Bullet, ("dispersion", 16));
