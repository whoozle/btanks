
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
#include "config.h"
#include "resource_manager.h"
#include "mrt/random.h"
#include "world.h"
#include "player_manager.h"

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
	virtual void emit(const std::string &event, Object * emitter = NULL);

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_type);
		s.add(_clone);
		s.add(_vel_backup);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_type);
		s.get(_clone);
		s.get(_vel_backup);
	}

private: 
	std::string _type;
	Alarm _clone;
	v2<float> _vel_backup;
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
			v2<float> vel;
			vel.fromDirection(d, dirs);
			Object * b = spawn(registered_name, animation, v2<float>::empty, vel);
			b->ttl = ttl * ttl_m;
			
			d = (dirs + getDirection() - 1) % dirs;
			vel.fromDirection(d, dirs);
			b = spawn(registered_name, animation, v2<float>::empty, vel);
			b->ttl = ttl * ttl_m;
		}
	}
}


void Bullet::calculate(const float dt) {
	if (_type == "mortar") {
		float idle, moving;
		getTimes(moving, idle);
		float real_ttl = ttl + moving + idle;
		GET_CONFIG_VALUE("objects.mortar-bullet.g", float, g, 2.0f);
		float v0 = real_ttl * g / 2;
		float t = real_ttl - ttl;
		_velocity = v2<float>(0, g * t - v0) + _vel_backup;
	}
	GET_CONFIG_VALUE("engine.auto-aim.enabled", bool, aa, true);
	if (aa && !PlayerManager->isClient() && _variants.has("auto-aim") && !_velocity.is0()) {
		if (!_clone.tick(dt)) 
			return;
		GET_CONFIG_VALUE("engine.auto-aim.range", float, aar, 128.0f);
		std::set<const Object *> objects;
		static std::set<std::string> targets;
		if (targets.empty()) {
			targets.insert("missile");	
			targets.insert("player");
			targets.insert("trooper");
			targets.insert("kamikaze");
			targets.insert("boat");
			targets.insert("helicopter");
			targets.insert("monster");
		}
		
		World->enumerateObjects(objects, this, aar, &targets);
		GET_CONFIG_VALUE("engine.auto-aim.minimum-cosine", float, min_cos, 0.939692f);
		const Object *target = NULL;
		
		for(std::set<const Object *>::const_iterator i = objects.begin(); i != objects.end(); ++i) {
			const Object *o = *i;
			if (hasSameOwner(o))
				continue;
			v2<float> rel = getRelativePosition(o);
			if (rel.is0())
				continue;

			v2<float> m = rel * _velocity;
			float cos_v = (m.x + m.y) / _velocity.length() / rel.length();
			if (cos_v >= min_cos) {
				min_cos = cos_v;
				target = o;
			}
		}
		if (target)	{
			if (_vel_backup.is0())
				_vel_backup = _velocity;
			_velocity = getRelativePosition(target);
			//LOG_DEBUG(("target: %s, cos: %g", target->animation.c_str(), min_cos));
		} else _velocity = _vel_backup;
	}
}

void Bullet::onSpawn() {
	if (_type == "dispersion") {
		GET_CONFIG_VALUE("objects.dispersion-bullet.clone-interval", float, ci, 0.1f);
		_clone.set(ci);
	} else {
		GET_CONFIG_VALUE("objects.dispersion-bullet.clone-interval", float, ci, 0.05f);
		_clone.set(ci);
	}

	play("shot", false);
	play("move", true);
	
	if (_type != "mortar")
		quantizeVelocity();
	_vel_backup = _direction = _velocity;
}

void Bullet::emit(const std::string &event, Object * emitter) {
	if (emitter != NULL && (emitter->classname == "smoke-cloud" || emitter->classname == "bullet") )
		return;

	v2<float> dpos;	
	if (event == "collision" || event == "death") {
		if (emitter) {
			dpos = getRelativePosition(emitter) / 2;
		}
		if (_type == "regular") {
			GET_CONFIG_VALUE("objects.explosion-downwards-z-override", int, edzo, 180)
			int z = (_velocity.y >= 0) ? edzo : 0;
			spawn("explosion", "explosion", dpos, v2<float>::empty, z);
		} else if (_type == "dirt") {
			spawn("dirt", "dirt", dpos);
		} else if (_type == "cannon") {
			spawn("cannon-explosion", "cannon-explosion", dpos);
		} else if (_type == "mortar") {
			spawn("mortar-explosion", "mortar-explosion", dpos);
		} else if (event == "collision" && _type == "ricochet" && (emitter == NULL || emitter->hp == -1)) {
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
			Object::emit(event, emitter);
			return;
		} else if (event == "collision" && ( 
			(_type == "ricochet" && emitter != NULL ) ||
			(_type == "dispersion")
			)
		) {
			GET_CONFIG_VALUE("objects.explosion-downwards-z-override", int, edzo, 180)
			int z = (_velocity.y >= 0) ? edzo : 0;
			spawn("explosion", "explosion", dpos, v2<float>::empty, z);			
		}
		Object::emit(event, emitter);
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

REGISTER_OBJECT("cannon-bullet", Bullet, ("cannon", 8));
REGISTER_OBJECT("mortar-bullet", Bullet, ("mortar", 1));
