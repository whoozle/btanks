
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
#include "mrt/random.h"

class Bullet : public Object {
public:
	Bullet(const std::string &type, const int dirs) : Object("bullet"), _type(type), _clone(false) {
		impassability = 1;
		piercing = true;
		setDirectionsNumber(dirs);
	}
	virtual void calculate(const float dt);
	virtual Object * clone() const;
	virtual void onSpawn();
	virtual void tick(const float dt);
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_type);
		_clone.serialize(s);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_type);
		_clone.deserialize(s);
	}

private: 
	std::string _type;
	Alarm _clone;
};

void Bullet::tick(const float dt) {
	Object::tick(dt);
	if (_type == "dispersion") {
		//LOG_DEBUG(("baaaaaaah!"));
		if (_clone.tick(dt)) {
			_clone.set(3600);
			//LOG_DEBUG(("%d clones...", getID()));
			
			GET_CONFIG_VALUE("objects.dispersion-bullet.ttl-multiplier", float, ttl_m, 0.8);
			const int dirs = getDirectionsNumber();
			int d = (getDirection() + 1) % dirs;
			v3<float> vel;
			vel.fromDirection(d, dirs);
			Object * b = spawn(registered_name, animation, v3<float>::empty, vel);
			b->ttl = ttl * ttl_m;
			b->setOwner(getOwner());
			
			d = (dirs + getDirection() - 1) % dirs;
			vel.fromDirection(d, dirs);
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
	
	quantizeVelocity();
}

void Bullet::emit(const std::string &event, BaseObject * emitter) {
	if (emitter != NULL && emitter->classname == "smoke-cloud")
		return;
	
	if (event == "collision" || event == "death") {
		v3<float> dpos;
		if (emitter) {
			dpos = getRelativePosition(emitter) / 2;
			dpos.z = 0;
		}
		if (_type == "regular") {
			spawn("explosion", "explosion", dpos);
		} else if (_type == "dirt") {
			spawn("dirt", "dirt", dpos);
		} else if (event == "collision" && _type == "ricochet" && emitter == NULL) {
			const int dirs = getDirectionsNumber();
			if (dirs != 16) 
				throw_ex(("%d-directional ricochet not supported yet.", dirs));
			
			//disown(); //BWAHAHHAHA!!!! 
			
			int dir = getDirection();

			int sign = (mrt::random(100) & 1) ? 1:-1;
			int d = sign * (mrt::random(dirs/4 - 1) + 1) ;
			dir = ( dir + d + dirs) % dirs;
			
			setDirection(dir);
			_velocity.fromDirection(dir, dirs);
			return;
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
REGISTER_OBJECT("ricochet-bullet", Bullet, ("ricochet", 16));
