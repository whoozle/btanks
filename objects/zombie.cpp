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
#include "alarm.h"
#include "config.h"
#include "mrt/random.h"
#include "ai/herd.h"
#include "ai/targets.h"

class BaseZombie : public Object {
public: 
	virtual Object * clone() const { return new BaseZombie(*this); }
	BaseZombie(const std::string &classname): Object(classname), _can_punch(true) {}
	
	void get_impassability_penalty(const float impassability, float &base, float &base_value, float &penalty) const {
		if (impassability > 0.2f) {
			base_value = 0.2f;
			base = 0;
			penalty = 0;
		}
	}

	virtual void on_spawn();
	virtual void tick(const float dt);
	virtual void emit(const std::string &event, Object * emitter = NULL);
	const bool take(const BaseObject *obj, const std::string &type) {
		return false;
	}

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_can_punch);
	}

	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_can_punch);
	}	

private:  
	bool _can_punch;	
};

void BaseZombie::on_spawn() {
	play("hold", true);
	disown();
}

void BaseZombie::tick(const float dt) {
	Object::tick(dt);

	if (_state.fire && get_state() != "punch") {
		_can_punch = true;
		play_now("punch");
		return;
	}

	if (_velocity.is0()) {
		if (get_state() != "hold") {
			cancel_all();
			play("hold", true);
		}
	} else {
		if (get_state() == "hold") {
			cancel_all();
			play("walk", true);
		}		
	}
}


void BaseZombie::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
		spawn("corpse(zombie-death)", "dead-zombie", v2<float>(), v2<float>());
	} else if (emitter != NULL && event == "collision") {
		if (get_state() != "punch" && emitter->registered_name != "zombie") {
			_state.fire = true;
		}	
		if (_state.fire && _can_punch && get_state_progress() >= 0.5 && get_state() == "punch" && emitter->registered_name != "zombie") {
			_can_punch = false;
			
			GET_CONFIG_VALUE("objects.zombie.damage", int, kd, 15);
		
			if (emitter && emitter->classname != "explosion") 
				emitter->add_damage(this, kd);
			
			return;
		}

	}
	Object::emit(event, emitter);
}




/*============================================
				ai zombie
============================================*/

class Zombie : public BaseZombie, public ai::Herd{
public:
	Zombie(const std::string &classname) : 
	BaseZombie(classname), _reaction(true) {
	}
	
	virtual void calculate(const float dt);

	virtual Object * clone() const;
	virtual void on_spawn();

	virtual void serialize(mrt::Serializator &s) const {
		BaseZombie::serialize(s);
		s.add(_reaction);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		BaseZombie::deserialize(s);
		s.get(_reaction);
	}	
	
	virtual void onIdle(const float dt);

	const int getComfortDistance(const Object *other) const;

private: 
	Alarm _reaction;
};

const int Zombie::getComfortDistance(const Object *other) const {
	GET_CONFIG_VALUE("objects.zombie.comfort-distance", int, cd, 120);
	return (other == NULL || other->classname == classname)?cd:-1; //fixme names if you want
}


void Zombie::onIdle(const float dt) {
	_state.fire = false;
	
	GET_CONFIG_VALUE("objects.zombie.targeting-range(stable)", int, trs, 600);
	GET_CONFIG_VALUE("objects.zombie.targeting-range(alerted)", int, tra, 900);
	int tt = (hp < max_hp)?tra:trs;

	ai::Herd::calculateV(_velocity, this, 0, tt);
}


void Zombie::calculate(const float dt) {
	v2<float> vel;
	int tt;
	
	if (is_driven())
		goto drive; 
	
	if (!_reaction.tick(dt))
		return;
	
	
	GET_CONFIG_VALUE("objects.zombie.targeting-range(stable)", int, trs, 600);
	GET_CONFIG_VALUE("objects.zombie.targeting-range(alerted)", int, tra, 900);
	tt = (hp < max_hp)?tra:trs;
	
	if (get_nearest(ai::Targets->monster, tt, _velocity, vel, false)) {
		if (_velocity.quick_length() > size.quick_length())
			_state.fire = false;
		
		_velocity.normalize();
		quantize_velocity();		
	} else {
		_state.fire = false;
		if (!_variants.has("no-herd"))
			onIdle(dt);
	}

drive:
	GET_CONFIG_VALUE("objects.zombie.rotation-time", float, rt, 0.1);

	calculate_way_velocity();
	limit_rotation(dt, rt, true, false);
	update_state_from_velocity();
}

void Zombie::on_spawn() {
	BaseZombie::on_spawn();

	float rt;
	
	Config->get("objects." + registered_name + ".reaction-time", rt, 0.5f);
	mrt::randomize(rt, rt/10);
	_reaction.set(rt);
}

Object* Zombie::clone() const  {
	return new Zombie(*this);
}

REGISTER_OBJECT("zombie", Zombie, ("monster"));
