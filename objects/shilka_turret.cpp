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
#include "registrar.h"
#include "alarm.h"
#include "config.h"
#include "ai/targets.h"
#include "zbox.h"

#define PIERCEABLE_PAIR(o1, o2) ((o1->piercing && o2->pierceable) || (o2->piercing && o1->pierceable))

class ShilkaTurret : public Object {
public:
	void tick(const float dt) {
		Object::tick(dt);

		bool play_fire = false;
		const bool fire_possible = _fire.tick(dt);

		if (_state.fire && fire_possible) {
			_fire.reset();
			
			static const std::string left_fire = "shilka-bullet-left";
			static const std::string right_fire = "shilka-bullet-right";
			std::string animation = "shilka-bullet-";
			animation += (_left_fire)?"left":"right";
			if (has_effect("dirt")) {
				if (getState().substr(0,4) == "fire") 
					cancel();
		
				static const std::string left_fire = "shilka-bullet-left";
				static const std::string right_fire = "shilka-bullet-right";
				std::string animation = "shilka-dirt-bullet-";
				animation += (_left_fire)?"left":"right";
				
				spawn("dirt-bullet", animation, v2<float>(), _direction);
	
				play_fire = true;
			} else if (has_effect("ricochet")) {
				spawn("ricochet-bullet(auto-aim)", "ricochet-bullet", v2<float>(), _direction);
				play_fire = true;
				_left_fire = ! _left_fire;
			} else if (has_effect("dispersion")) {
			/*
				if (special_fire_possible) {
					_special_fire.reset();
					spawn("dispersion-bullet", "dispersion-bullet", v2<float>(), _direction);
					play_fire = true;
				};
			*/
				_left_fire = ! _left_fire;
				goto skip_left_toggle;
			} else { 
				//LOG_DEBUG(("%g %g", _direction.x, _direction.y));
				spawn("shilka-bullet", animation, v2<float>(), _direction);
				play_fire = true;
				_left_fire = ! _left_fire;
			}
		}

skip_left_toggle:
	if (play_fire) {
		if (getState().substr(0,4) == "fire") 
			cancel();
		
		playNow(_left_fire?"fire-left":"fire-right");
	}


	} //end of tick()
	
	void calculate(const float dt) {
		if (_parent == NULL)
			throw_ex(("turret is only operable attached to shilka "));
					
		v2<float> pos, vel;
		std::set<const Object *> objects;
		enumerateObjects(objects, getWeaponRange("shilka-bullet"), &ai::Targets->troops);

		int dirs = getDirectionsNumber();
		//int parent_dir = _parent->getDirection();
		//(_parent->getDirection() - _parent->getDirectionsNumber() / 2) * getDirectionsNumber() / _parent->getDirectionsNumber();

		const Object *target = NULL;
		v2<float> target_pos;
		for(std::set<const Object *>::iterator i = objects.begin(); i != objects.end(); ++i) {
			const Object *o = *i;
			if (o->getID() == getID() || o->impassability == 0 || 
				PIERCEABLE_PAIR(this, o) || !ZBox::sameBox(getZ(), o->getZ()) || hasSameOwner(o) ||
				o->has_effect("invulnerability")
				)
				continue;
			
			pos = getRelativePosition(o);
			if (target == NULL || pos.quick_length() < target_pos.quick_length()) {
				target = o;
				target_pos = pos;
			}
			//LOG_DEBUG(("%s <- dir: %d, parent_dir: %d (%g, %g)", o->animation.c_str(), dir, parent_dir, pos.x, pos.y));
		}
		
		target_pos.normalize();
		int dir = target_pos.getDirection(dirs) - 1;

		if (target == NULL || dir < 0) {
			Object::calculate(dt);
			return;
		}

		_direction = target_pos;		
		setDirection(dir);
	}

	void onSpawn() {
		play("hold", true);
		GET_CONFIG_VALUE("objects.shilka.fire-rate", float, fr, 0.2);
		_fire.set(fr);
	}

	ShilkaTurret() : Object("turrel"), _reaction(0.1f, true), _fire(false), _left_fire(false) {
		impassability = 0;
		hp = -1;
		setDirectionsNumber(16);
		pierceable = true;
	}

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_fire);
		s.add(_left_fire);
	}

	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_fire);	
		s.get(_left_fire);
	}
	
	virtual Object * clone() const { return new ShilkaTurret(*this); }
	

private:
	Alarm _reaction, _fire;
	bool _left_fire;
};

REGISTER_OBJECT("shilka-turret", ShilkaTurret, ());
