
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

#include "object.h"
#include "config.h"
#include "registrar.h"

class MortarBullet : public Object {
public:
	Object *clone() const { return new MortarBullet(*this); }
	
	MortarBullet() : Object("bullet") {
		impassability = -1;
		piercing = true;
		setDirectionsNumber(1);
	}
	
	void onSpawn() {
		play("shot", false);
		play("move", true);
	
		_vel_backup = _direction = _velocity;
	}
	
	void calculate(const float dt) {
		float idle, moving;
		getTimes(moving, idle);
		float real_ttl = ttl + moving + idle;
		GET_CONFIG_VALUE("objects.mortar-bullet.g", float, g, 2.0f);
		float v0 = real_ttl * g / 2;
		float t = real_ttl - ttl;
		_velocity = v2<float>(0, g * t - v0) + _vel_backup;

		float progress = ttl / real_ttl;
		bool fly = (progress >= 0.3f && progress < 0.7f);
		if (fly && getZ() != 999) {
			setZ(999);
		} else if (!fly && getZ() != 201) {
			setZ(201);
		}
	}
	
	void emit(const std::string &event, Object * emitter) {
		if (emitter != NULL && (emitter->classname == "smoke-cloud" || emitter->classname == "bullet") )
			return;
			
		bool collision = event == "collision";
		bool mortar = registered_name == "mortar-bullet";

		if (collision) {
			float idle, moving;
			getTimes(moving, idle);
			float progress = ttl / (ttl + moving + idle);
			bool fly = (progress >= 0.3f && progress < 0.7f);
			//LOG_DEBUG(("fly: %c, emitter: %s", fly?'+':'-', emitter != NULL?emitter->animation.c_str(): "-"));
			if (fly && (emitter == NULL || (emitter->speed == 0 && emitter->registered_name != "sandworm-head")))
				return;
		}
		
		if (collision || event == "death") {
			v2<float> dpos;
			if (emitter) {
				dpos = getRelativePosition(emitter) / 2;
			} 
			
			if (mortar) 
				spawn("mortar-explosion", "mortar-explosion", dpos);
			else 
				spawn("grenade-explosion", "grenade-explosion", dpos);
			
			Object::emit("death", emitter);
			return;
		} 
		Object::emit(event, emitter);
	}

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_vel_backup);
	}

	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_vel_backup);
	}


private: 
	v2<float> _vel_backup;
};

REGISTER_OBJECT("mortar-bullet", MortarBullet, ());
REGISTER_OBJECT("grenade", MortarBullet, ());
