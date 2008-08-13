
/* Battle Tanks Game
 * Copyright (C) 2006-2008 Battle Tanks team
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
#include "config.h"
#include "registrar.h"
#include "mrt/random.h"
#include "player_manager.h"
#include "ai/targets.h"

class Bullet : public Object {
public:
	Bullet(const std::string &type, const int dirs) : 
	Object("bullet"), _type(type), _clone(false), _guard_interval(false), _guard_state(true) {
		impassability = 1;
		piercing = true;
		set_directions_number(dirs);
	}
	virtual void calculate(const float dt);
	virtual Object * clone() const;
	virtual void on_spawn();
	virtual void tick(const float dt);
	virtual void emit(const std::string &event, Object * emitter = NULL);

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_type);
		s.add(_clone);
		s.add(_guard_interval);
		s.add(_vel_backup);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_type);
		s.get(_clone);
		s.get(_guard_interval);
		s.get(_vel_backup);
	}

private: 
	std::string _type;
	Alarm _clone, _guard_interval;
	v2<float> _vel_backup;
	
	//do not serialize it
	bool _guard_state;
};

void Bullet::tick(const float dt) {
	Object::tick(dt);
	if (_type == "dispersion") {
		//LOG_DEBUG(("baaaaaaah!"));
		if (_clone.tick(dt)) {
			_clone.set(3600);
			//LOG_DEBUG(("%d clones...", get_id()));
			
			GET_CONFIG_VALUE("objects.dispersion-bullet.ttl-multiplier", float, ttl_m, 0.8);
			const int dirs = get_directions_number();
			int d = (get_direction() + 1) % dirs;
			v2<float> vel;
			vel.fromDirection(d, dirs);
			Object * b = spawn(registered_name + "(no-sound)", animation, v2<float>(), vel);
			b->ttl = ttl * ttl_m;
			
			d = (dirs + get_direction() - 1) % dirs;
			vel.fromDirection(d, dirs);
			b = spawn(registered_name + "(no-sound)", animation, v2<float>(), vel);
			b->ttl = ttl * ttl_m;
		}
	} else if (_type == "ricochet") {
		if (_guard_interval.tick(dt))
			_guard_state = true;
	}
}


void Bullet::calculate(const float dt) {
	GET_CONFIG_VALUE("engine.auto-aim.enabled", bool, aa, true);
	if (aa && _variants.has("auto-aim") && !_velocity.is0()) {
		if (!_clone.tick(dt)) 
			return;
		GET_CONFIG_VALUE("engine.auto-aim.range", float, aar, 192.0f);
		std::set<const Object *> objects;
		
		enumerate_objects(objects, aar, &ai::Targets->troops);
		GET_CONFIG_VALUE("engine.auto-aim.minimum-cosine", float, min_cos, 0.9848f); //~cos(10')
		const Object *target = NULL;
		
		for(std::set<const Object *>::const_iterator i = objects.begin(); i != objects.end(); ++i) {
			const Object *o = *i;
			if (has_same_owner(o) || o->pierceable || o->impassability == 0 || o->hp <= 0)
				continue;
			v2<float> rel = get_relative_position(o);
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
			_velocity = get_relative_position(target);
			//LOG_DEBUG(("target: %s, cos: %g", target->animation.c_str(), min_cos));
		} else _velocity = _vel_backup;
	}
}

void Bullet::on_spawn() {
	if (_type == "ricochet") {
		GET_CONFIG_VALUE("objects.ricochet.guard-interval", float, gi, 0.1f);
		_guard_interval.set(gi);
	}
	if (_type == "dispersion") {
		_variants.remove("auto-aim"); //no auto aim! 
		GET_CONFIG_VALUE("objects.dispersion-bullet.clone-interval", float, ci, 0.1f);
		_clone.set(ci);
		if (!_variants.has("no-sound")) 
			play_sound("dispersion-bullet", false);
	} else {
		GET_CONFIG_VALUE("engine.auto-aim.checking-interval", float, ci, 0.05f);
		_clone.set(ci);
	}

	play("shot", false);
	play("move", true);
	
	quantize_velocity();
	_vel_backup = _direction = _velocity;
}

void Bullet::emit(const std::string &event, Object * emitter) {
	if (emitter != NULL && (emitter->classname == "smoke-cloud" || emitter->classname == "bullet") )
		return;

	v2<float> dpos;
	int dirs = get_directions_number();
	if (dirs == 4 || dirs == 8 || dirs == 16) {
		dpos.fromDirection(get_direction(), dirs);
		dpos *= size.length() / 2;
	}
	if (event == "collision" || event == "death") {
/*		if (emitter) {
			dpos = get_relative_position(emitter) / 2;
		} 
*/
		if (_type == "regular") {
			GET_CONFIG_VALUE("objects.explosion-downwards-z-override", int, edzo, 180);
			int z = (_velocity.y >= 0) ? edzo : 0;
			spawn("explosion", "explosion", dpos, v2<float>(), z);
		} else if (emitter != NULL && _type == "stun") {
			if (emitter->classname == "monster")
				return;
			
			float sd;
			Config->get("objects." + registered_name + ".stun-duration", sd, 3.0f);
			emitter->add_effect("stunned", sd);
		} else if (_type == "dirt") {
			spawn("dirt", "dirt", dpos);
		} else if (_type == "cannon") {
			spawn("cannon-explosion", "cannon-explosion", dpos);
		} else if (event == "collision" && _type == "ricochet" && (emitter == NULL || emitter->hp == -1)) {
			if (!_guard_state)
				return;	

			_guard_state = false;
			_guard_interval.reset();

			const int dirs = get_directions_number();
			if (dirs != 16) 
				throw_ex(("%d-directional ricochet not supported yet.", dirs));
			//LOG_DEBUG(("ricocheting..."));
			
			//disown(); //BWAHAHHAHA!!!! 
			
			int dir = get_direction();

			int sign = (mrt::random(103) % 3) - 1;
			int d = sign * (mrt::random(dirs/4 - 1) + 1) ;
			dir = ( dir + d + dirs + dirs / 2) % dirs;
			
			set_direction(dir);
			_velocity.fromDirection(dir, dirs);
			_direction = _velocity;
			_vel_backup = _velocity;

			//GET_CONFIG_VALUE("objects.explosion-downwards-z-override", int, edzo, 180);
			//int z = (_velocity.y >= 0) ? edzo : 0;
			//spawn("explosion", "explosion", dpos, v2<float>(), z);
			//LOG_DEBUG(("new velocity: %g %g", _velocity.x, _velocity.y));
			return;
		} else if (event == "collision" && ( 
			(_type == "ricochet" && emitter != NULL ) ||
			(_type == "dispersion")
			)
		) {
			GET_CONFIG_VALUE("objects.explosion-downwards-z-override", int, edzo, 180);
			int z = (_velocity.y >= 0) ? edzo : 0;
			spawn("explosion", "explosion", dpos, v2<float>(), z);			
		}
		if (event != "death")
			Object::emit(event, emitter);
		Object::emit("death", emitter);
	} else Object::emit(event, emitter);
}


Object* Bullet::clone() const  {
	return new Bullet(*this);
}

REGISTER_OBJECT("bullet", Bullet, ("regular", 8));
REGISTER_OBJECT("shilka-bullet", Bullet, ("regular", 16));
REGISTER_OBJECT("dirt-bullet", Bullet, ("dirt", 8));
REGISTER_OBJECT("machinegunner-bullet", Bullet, ("regular", 16));
REGISTER_OBJECT("vehicle-machinegunner-bullet", Bullet, ("regular", 16));

REGISTER_OBJECT("dispersion-bullet", Bullet, ("dispersion", 16));
REGISTER_OBJECT("ricochet-bullet", Bullet, ("ricochet", 16));

REGISTER_OBJECT("cannon-bullet", Bullet, ("cannon", 8));

REGISTER_OBJECT("slime-acid", Bullet, ("stun", 8));
