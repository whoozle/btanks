
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
#include "registrar.h"
#include "config.h"
#include "zbox.h"
#include "math/binary.h"
#include "mrt/random.h"
#include "world.h"
#include "ai/targets.h"

class Missile : public Object {
public:
	std::string type;
	Missile(const std::string &type) : Object("missile"), type(type), _reaction(true) {
		piercing = true;
		set_directions_number(16);
	}
	virtual void add_damage(BaseObject *from, const int hp, const bool emitDeath = true) {}
	virtual void calculate(const float dt);
	virtual Object * clone() const;
	virtual void emit(const std::string &event, Object * emitter = NULL);
	void on_spawn();

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(type);
		s.add(_reaction);
		s.add(_target);
		s.add(_moving_time);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(type);
		s.get(_reaction);
		s.get(_target);
		s.get(_moving_time);
	}
private: 
	Alarm _reaction;
	v2<float> _target;
	float _moving_time;
};

void Missile::on_spawn() {
	if (type == "guided" || type == "stun") {
		GET_CONFIG_VALUE("objects.guided-missile.reaction-time", float, rt, 0.05);
		mrt::randomize(rt, rt / 10);
		_reaction.set(rt);
	}

	play("main", true);
	if (type != "boomerang") {
		Object *_fire = add("fire", "single-pose", "missile-fire", v2<float>(), Centered);
		_fire->set_directions_number(16);
		_fire->impassability = 0;
	} 
	
	play_sound(type + "-missile", false);
	quantize_velocity();
	_target = _velocity;
}

void Missile::calculate(const float dt) {
	if (type == "guided" || type == "stun") {
		v2<float> pos, vel;
		if (_reaction.tick(dt) && get_nearest(type == "stun"? ai::Targets->players_and_monsters: ai::Targets->troops, math::min(ttl * speed, 800.0f), pos, vel, true)) {
			float est_t = pos.length() / speed;
			if (est_t > 1)
				est_t = 1;
			_target = _velocity = pos + vel * est_t;
		} else { 
			_velocity = _target;
		}
		//LOG_DEBUG(("%d: velocity: %g %g", get_id(), _velocity.x, _velocity.y));

		GET_CONFIG_VALUE("objects." + type + "-missile.rotation-time", float, rotation_time, 0.2);
		limit_rotation(dt, rotation_time, false, false);
	} else if (type == "boomerang") {
		GET_CONFIG_VALUE("objects.boomerang.rotation-speed", float, rs, 30);
		int dir = ((int)(_moving_time * rs)) % 8;
		set_direction(dir);

		const Object *leader = World->getObjectByID(get_summoner());
		if (leader == NULL) {
			return;
		}
		if (!ZBox::sameBox(leader->get_z(), get_z())) {
			set_zbox(leader->get_z());
		}
		
		_direction.normalize();
		//LOG_DEBUG(("direction %g %g", _direction.x, _direction.y));
		_velocity.normalize();
		v2<float> lpos = get_relative_position(leader);

		GET_CONFIG_VALUE("objects.boomerang.radius", float, r, 3000);
		GET_CONFIG_VALUE("objects.boomerang.turning-speed", float, ts, 0.1);
		_velocity = _velocity * r + lpos;
		_velocity.normalize();
		
		lpos.quantize16();
		dir = lpos.get_direction16();
		if (dir) {
			lpos.fromDirection(dir % 16, 16);
			_velocity += lpos * ts;
		}
	}
	if (!_velocity.is0())
		_moving_time += dt;
	else 
		_moving_time = 0;
}

void Missile::emit(const std::string &event, Object * emitter) {
	if (event == "collision") {
		if (type == "boomerang") {
			if (emitter == NULL || emitter->hp == -1) {
				if (!playing_sound("boomerang-hit"))
					play_sound("boomerang-hit", false);
				_velocity = -_velocity;
				return;
			}
		}
		if (emitter != NULL) { 
			if (type == "stun") {
				GET_CONFIG_VALUE("objects.stun-missile.stun-duration", float, sd, 5);
				if (emitter != NULL)
					emitter->add_effect("stunned", sd);
			}
			if (emitter->classname == "smoke-cloud" && type != "smoke")
				return;
		} 
		emit("death", emitter);
	} if (event == "death") {
		fadeout_sound(type + "-missile");
		if (type == "smoke") {
			GET_CONFIG_VALUE("objects.smoke-cloud-downwards-z-override", int, csdzo, 350);
			int z = (_velocity.y > 0)? csdzo: 0;
			//LOG_DEBUG(("edzo = %d", edzo));
			spawn("smoke-cloud", "smoke-cloud", v2<float>(), v2<float>(), z);
		} else if (type == "nuke" || type == "mutagen") {
			Object *o = World->getObjectByID(get_summoner()); //player
			v2<float> dpos;
			if (o != NULL) {
				dpos = o->get_relative_position(this);
			}
			Object * e = (o != NULL? o: this)->spawn(type + "-explosion", type + "-explosion", dpos, v2<float>());
		
			e->disown();
		} else {
			v2<float> dpos;
			
			GET_CONFIG_VALUE("objects.explosion-downwards-z-override", int, edzo, 180);
			int z = (_velocity.y >= 0)?edzo: 0;

			spawn("explosion", "missile-explosion", dpos, v2<float>(), z);
		}
		Object::emit(event, emitter);
	} else Object::emit(event, emitter);
}


Object* Missile::clone() const  {
	return new Missile(*this);
}

REGISTER_OBJECT("guided-missile", Missile, ("guided"));
REGISTER_OBJECT("thrower-missile", Missile, ("guided"));
REGISTER_OBJECT("dumb-missile", Missile, ("dumb"));
REGISTER_OBJECT("smoke-missile", Missile, ("smoke"));
REGISTER_OBJECT("nuke-missile", Missile, ("nuke"));
REGISTER_OBJECT("boomerang-missile", Missile, ("boomerang"));
REGISTER_OBJECT("stun-missile", Missile, ("stun"));
REGISTER_OBJECT("mutagen-missile", Missile, ("mutagen"));
