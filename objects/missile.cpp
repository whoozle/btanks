
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
#include "config.h"
#include "world.h"

class Missile : public Object {
public:
	std::string type;
	Missile(const std::string &type) : Object("missile"), type(type) {
		piercing = true;
		setDirectionsNumber(16);
	}
	virtual void addDamage(BaseObject *from, const int hp, const bool emitDeath = true) {}
	virtual void calculate(const float dt);
	virtual Object * clone() const;
	virtual void emit(const std::string &event, Object * emitter = NULL);
	void onSpawn();

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(type);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(type);
	}

};

void Missile::onSpawn() {
	play("main", true);
	if (type != "boomerang") {
		Object *_fire = spawnGrouped("single-pose", "missile-fire", v3<float>::empty, Centered);
		_fire->setDirectionsNumber(16);
		_fire->impassability = 0;
		add("fire", _fire);
	}
	
	quantizeVelocity();
}

void Missile::calculate(const float dt) {
	if (type == "guided" || type == "stun") {
		std::vector<std::string> targets;
		targets.push_back("player");
		if (type != "stun") {
			targets.push_back("trooper");
			targets.push_back("kamikaze");
		}
	
		v3<float> pos, vel;
	
		if (getNearest(targets, pos, vel)) {
			_velocity = pos;
		}

		GET_CONFIG_VALUE("objects." + type + "-missile.rotation-time", float, rotation_time, 0.2);
		limitRotation(dt, rotation_time, false, false);
	} else if (type == "boomerang") {
		GET_CONFIG_VALUE("objects.boomerang.rotation-speed", float, rs, 30);
		int dir = ((int)(_moving_time * rs)) % 8;
		setDirection(dir);

		Object *leader = World->getObjectByID(getOwner());
		if (leader == NULL) {
			return;
		}
		_direction.normalize();
		//LOG_DEBUG(("direction %g %g", _direction.x, _direction.y));
		_velocity.normalize();
		v3<float> lpos = getRelativePosition(leader);

		GET_CONFIG_VALUE("objects.boomerang.radius", float, r, 3000);
		GET_CONFIG_VALUE("objects.boomerang.turning-speed", float, ts, 0.1);
		_velocity = _velocity * r + lpos;
		_velocity.normalize();
		
		lpos.quantize16();
		dir = lpos.getDirection16();
		if (dir) {
			lpos.fromDirection(dir % 16, 16);
			_velocity += lpos * ts;
		}
	}
}

void Missile::emit(const std::string &event, Object * emitter) {
	if (event == "collision") {
		if (emitter != NULL) { 
			if (type == "stun") {
				GET_CONFIG_VALUE("objects.stun-missile.stun-duration", float, sd, 5);
				if (emitter != NULL)
					emitter->addEffect("stunned", sd);
			}
			if (emitter->classname == "smoke-cloud" && type != "smoke")
				return;
		}
		emit("death", emitter);
	} if (event == "death" && type == "smoke") {
		spawn("smoke-cloud", "smoke-cloud");
		Object::emit(event, emitter);
	} else if (event == "death" && type == "nuke") {
		//Object *e = 
		Object *o = World->getObjectByID(getRealOwner()); //player
		v3<float> dpos;
		if (o != NULL) {
			dpos = o->getRelativePosition(this);
			dpos.z = 0;
		}
			
		Object * e = World->spawn(o != NULL?o:this, "nuclear-explosion", "nuclear-explosion", dpos, v3<float>::empty);
		e->disown();
		Object::emit(event, emitter);
	} else if (event == "death") {
		v3<float> dpos;
		
		GET_CONFIG_VALUE("objects.explosion-downwards-z-override", int, edzo, 180)
		if (_velocity.y >= 0)
			dpos.z = edzo;

		spawn("explosion", "missile-explosion", dpos);
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
