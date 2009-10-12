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
#include "registrar.h"
#include "alarm.h"
#include "config.h"
#include "ai/targets.h"
#include "zbox.h"
#include "player_manager.h"
#include "mrt/random.h"

#define PIERCEABLE_PAIR(o1, o2) ((o1->piercing && o2->pierceable) || (o2->piercing && o1->pierceable))

class ShilkaTurret : public Object {
public:
	void tick(const float dt) {
		Object::tick(dt);
		if (_parent == NULL)
			throw_ex(("turret is only operable attached to shilka "));

		bool play_fire = false;
		const bool fire_possible = _fire.tick(dt);
		const bool alt_fire_possible = _special_fire.tick(dt);


		if (_state.alt_fire && alt_fire_possible) {
			_special_fire.reset();
			if (_parent->has_effect("dirt")) {
				if (get_state().substr(0,4) == "fire") 
					cancel();
		
				static const std::string left_fire = "shilka-bullet-left";
				static const std::string right_fire = "shilka-bullet-right";
				std::string animation = "shilka-dirt-bullet-";
				animation += (_left_fire)?"left":"right";
				
				_parent->spawn("dirt-bullet", animation, v2<float>(), _direction);
	
				play_fire = true;
			} 
		}

		if (_state.fire && fire_possible) {
			_fire.reset();
			
			if (_parent->has_effect("ricochet")) {
				_parent->spawn("ricochet-bullet(auto-aim)", "ricochet-bullet", v2<float>(), _direction);
				play_fire = true;
				_left_fire = ! _left_fire;
			} else if (_parent->has_effect("dispersion")) {
			
				if (alt_fire_possible) {
					_special_fire.reset();
					_parent->spawn("dispersion-bullet", "dispersion-bullet", v2<float>(), _direction);
					play_fire = true;
					_left_fire = ! _left_fire;
				};
			
			} else { 
				static const std::string left_fire = "shilka-bullet-left";
				static const std::string right_fire = "shilka-bullet-right";
				std::string animation = "shilka-bullet-";
				animation += (_left_fire)?"left":"right";
				
				_parent->spawn("shilka-bullet", animation, v2<float>(), _direction);
				play_fire = true;
				_left_fire = ! _left_fire;
			}
		}

	if (play_fire) {
		if (get_state().substr(0,4) == "fire") 
			cancel();
		
		play_now(_left_fire?"fire-left":"fire-right");
	}


	} //end of tick()
	
	void calculate(const float dt) {
		if (!_reaction.tick(dt)) 
			return;
		
		if (_parent == NULL)
			throw_ex(("turret is only operable attached to shilka "));

		if (_parent->disable_ai && PlayerManager->get_slot_by_id(_parent->get_id()) == NULL) {
			Object::calculate(dt);
			return;
		}
					
		v2<float> pos, vel;
		std::set<const Object *> objects;
		_parent->enumerate_objects(objects, getWeaponRange("shilka-bullet"), &ai::Targets->troops);

		int dirs = get_directions_number();
		//int parent_dir = _parent->get_direction();
		//(_parent->get_direction() - _parent->get_directions_number() / 2) * get_directions_number() / _parent->get_directions_number();

		const Object *target = NULL;
		v2<float> target_pos;
		for(std::set<const Object *>::iterator i = objects.begin(); i != objects.end(); ++i) {
			const Object *o = *i;
			if (o->get_id() == _parent->get_id() || o->impassability == 0 || o->hp <= 0 ||
				PIERCEABLE_PAIR(_parent, o) || !ZBox::sameBox(_parent->get_z(), o->get_z()) || _parent->has_same_owner(o) ||
				o->has_effect("invulnerability")
				)
				continue;
			
			pos = get_relative_position(o);
			if (target == NULL || pos.quick_length() < target_pos.quick_length()) {
				target = o;
				target_pos = pos;
			}
			//LOG_DEBUG(("%s <- dir: %d, parent_dir: %d (%g, %g)", o->animation.c_str(), dir, parent_dir, pos.x, pos.y));
		}
		
		target_pos.normalize();
		int dir = target_pos.get_direction(dirs) - 1;

		if (target == NULL || dir < 0) {
			Object::calculate(dt);
			return;
		}

		_direction = target_pos;		
		set_direction(dir);
	}

	void on_spawn() {
		play("hold", true);
		GET_CONFIG_VALUE("objects.shilka.fire-rate", float, fr, 0.2f);
		_fire.set(fr);
		GET_CONFIG_VALUE("objects.shilka.special-fire-rate", float, sfr, 0.4f);
		_special_fire.set(sfr);
	}

	ShilkaTurret() : Object("turrel"), _reaction(true), _fire(false), _special_fire(false), _left_fire(false) {
		impassability = 0;
		hp = -1;
		set_directions_number(16);
		pierceable = true;
		float rt = 0.1f;
		mrt::randomize(rt, rt/10);
		_reaction.set(rt);
	}

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_reaction);	
		s.add(_fire);
		s.add(_special_fire);
		s.add(_left_fire);
	}

	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_reaction);	
		s.get(_fire);	
		s.get(_special_fire);
		s.get(_left_fire);
	}
	
	virtual Object * clone() const { return new ShilkaTurret(*this); }
	

private:
	Alarm _reaction, _fire, _special_fire;
	bool _left_fire;
};

REGISTER_OBJECT("shilka-turret", ShilkaTurret, ());
