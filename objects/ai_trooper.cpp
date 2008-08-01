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

#include "trooper.h"
#include "ai/base.h"
#include "ai/herd.h"
#include "ai/targets.h"
#include "ai/old_school.h"
#include "config.h"
#include "registrar.h"
#include "mrt/random.h"
#include "special_owners.h"

class AITrooper : public Trooper, private ai::Herd, private ai::Base, ai::OldSchool {
public:
	AITrooper(const std::string &object, const bool aim_missiles) : 
		Trooper("trooper", object), _reaction(true), _target_dir(-1), _aim_missiles(aim_missiles) {}
	virtual void on_spawn();
	virtual void serialize(mrt::Serializator &s) const {
		Trooper::serialize(s);
		ai::Base::serialize(s);
		ai::OldSchool::serialize(s);
		s.add(_reaction);
		s.add(_target_dir);
		s.add(_aim_missiles);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Trooper::deserialize(s);
		ai::Base::deserialize(s);
		ai::OldSchool::deserialize(s);
		s.get(_reaction);
		s.get(_target_dir);
		s.get(_aim_missiles);
	}
	virtual void calculate(const float dt);
	virtual Object* clone() const;

	virtual void onIdle(const float dt);
	
private: 
	const bool validateFire(const int idx) {
		if (idx == 0) 
			return canFire();
		return true;
	}
	
	virtual const int getComfortDistance(const Object *other) const;

	Alarm _reaction;
	int _target_dir;
	bool _aim_missiles;
};

const int AITrooper::getComfortDistance(const Object *other) const {
	GET_CONFIG_VALUE("objects.ai-trooper.comfort-distance", int, cd, 80);
	return (other == NULL || other->classname == "trooper" || other->classname == "kamikaze")?cd:-1;
}

#include "world.h"

void AITrooper::onIdle(const float dt) {
	int summoner = get_summoner();
	if (_variants.has("old-school")) {
		ai::OldSchool::calculateV(_velocity, this);
	} else if ((summoner != 0 && summoner != OWNER_MAP) || _variants.has("herd")) {
		Object *parent = World->getObjectByID(summoner);
		if (parent != NULL) {
			v2<float> dpos = get_relative_position(parent);
			float dist = dpos.length();
			//LOG_DEBUG(("%d: %s: summoner distance: %g", get_id(), animation.c_str(), dist));
			if (dist > 800) {
				//teleportation! 
				LOG_DEBUG(("%d: %s: teleports from distance: %g", get_id(), animation.c_str(), dist));
				v2<float> dir;
				dir.fromDirection(get_id() % 16, 16);
				dir *= (parent->size.x + parent->size.x) / 3;

				World->teleport(this, parent->get_center_position() + dir);
				set_zbox(parent->get_z());
				return;
			}
			
		}
		float range = getWeaponRange(_object);
		ai::Herd::calculateV(_velocity, this, summoner, range);
	} else {
		_velocity.clear();
	}
	_state.fire = false;

	GET_CONFIG_VALUE("objects.ai-trooper.rotation-time", float, rt, 0.05);
	calculate_way_velocity();
	limit_rotation(dt, rt, true, false);
	update_state_from_velocity();	
}

void AITrooper::on_spawn() {
	ai::Base::on_spawn(this);
	ai::OldSchool::on_spawn(this);
	GET_CONFIG_VALUE("objects.ai-trooper.reaction-time", float, rt, 0.15f);
	mrt::randomize(rt, rt / 10);
	//LOG_DEBUG(("rt = %g", rt));
	_reaction.set(rt);	
	Trooper::on_spawn();
	if (_variants.has("monstroid"))
		classname = "monster";
}

Object* AITrooper::clone() const  {
	return new AITrooper(*this);
}


void AITrooper::calculate(const float dt) {
	//calculate_way_velocity();
	//LOG_DEBUG(("calculate"));
	if (_target_dir != -1 && has_effect("panic")) {
		//LOG_DEBUG(("panic: %d", _target_dir));
		_velocity.fromDirection(_target_dir, get_directions_number());
	
		GET_CONFIG_VALUE("objects.ai-trooper.rotation-time", float, rt, 0.05f);
		limit_rotation(dt, rt, true, false);
		update_state_from_velocity();
		return;
	}
	
	if (!_reaction.tick(dt) || is_driven()) {
		calculate_way_velocity();
		return;
	}

	{/*
		static std::set<std::string> bullets; 
		if (bullets.empty()) {
			bullets.insert("bullet");
			bullets.insert("missile");
		}
		//checking for a bullets 
		v2<float> pos, vel;
		float r = speed * 5.0f; 

		if (get_nearest(bullets, r, pos, vel, false)) {
			float ct = get_collision_time(pos, vel, 16);
			//LOG_DEBUG(("bullet at %g %g, est: %g", pos.x, pos.y, ct));
			if (ct > 0 && ct > 0.15f) {
				v2<float> dpos = -(pos + vel * ct);
				//LOG_DEBUG(("AAAAAAA!!"));
				dpos.normalize();
				int dirs = get_directions_number(), escape = dpos.get_direction(dirs) - 1;
				if (escape >= 0) {
					_target_dir = escape;
					set_direction(escape);
					_velocity.fromDirection(_target_dir, get_directions_number());
					_direction.fromDirection(_target_dir, get_directions_number());
					addEffect("panic", ct);
					return;
				}
			}
		}
		*/
	}

	float range = getWeaponRange(_object);
	
	//LOG_DEBUG(("variants: %s", _variants.dump().c_str()));
	_target_dir = get_target_position(_velocity,
			_variants.has("monstroid")? ai::Targets->monster: 
			(
			_variants.has("trainophobic")? 
				(_aim_missiles? ai::Targets->troops_train_and_missiles: ai::Targets->troops_and_missiles): 
				(_aim_missiles? ai::Targets->troops_and_missiles: ai::Targets->troops)
			), 
		range);
	if (_target_dir >= 0) {
		//LOG_DEBUG(("target: %g %g %g", tp.x, tp.y, tp.length()));
		/*
		Way way;
		if (find_path(tp, way)) {
		set_way(way);
			calculate_way_velocity();
		}
		*/
		if (_velocity.length() >= 9) {
			quantize_velocity();
			_direction.fromDirection(get_direction(), get_directions_number());
			_state.fire = false;
		} else {
			_velocity.clear();
			set_direction(_target_dir);
			//LOG_DEBUG(("%d", _target_dir));
			_direction.fromDirection(_target_dir, get_directions_number());
			_state.fire = true;
		}
	
	} else {
		_velocity.clear();
		_target_dir = -1;
		onIdle(dt);
		_state.fire = false;
	}
}
//==============================================================================
class TrooperInWatchTower : public Trooper, private ai::Base {
public: 
	TrooperInWatchTower(const std::string &object, const bool aim_missiles) : 
		Trooper("trooper", object), _reaction(true), _aim_missiles(aim_missiles) {}
	virtual Object * clone() const { return new TrooperInWatchTower(*this); }
	
	virtual void on_spawn() { 
		ai::Base::on_spawn(this);
	
		GET_CONFIG_VALUE("objects.ai-trooper.reaction-time", float, rt, 0.15f);
		mrt::randomize(rt, rt/10);
		_reaction.set(rt);
	
		Trooper::on_spawn();
	}

	virtual void serialize(mrt::Serializator &s) const {
		Trooper::serialize(s);
		ai::Base::serialize(s);
		s.add(_reaction);
		s.add(_aim_missiles);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Trooper::deserialize(s);
		ai::Base::deserialize(s);
		s.get(_reaction);
		s.get(_aim_missiles);
	}
	
	virtual void calculate(const float dt) {
		if (!_reaction.tick(dt))
			return;
		
		float range = getWeaponRange(_object);
		//LOG_DEBUG(("range = %g", range));

		_state.fire = false;

		const Object * result = NULL;
		float dist = -1;
		
		std::set<const Object *> objects;
		enumerate_objects(objects, range, 
			&(_variants.has("trainophobic")? (_aim_missiles? ai::Targets->troops_train_and_missiles: ai::Targets->troops_and_missiles): (_aim_missiles? ai::Targets->troops_and_missiles: ai::Targets->troops))
		);
		for(std::set<const Object *>::const_iterator i = objects.begin(); i != objects.end(); ++i) {
			const Object *target = *i;
			if (has_same_owner(target) || target->ai_disabled() || target->impassability == 0 || target->pierceable || target->hp <= 0)
				continue;
			
			v2<float> dpos = get_relative_position(target);
			if (check_distance(get_center_position(), target->get_center_position(), get_z(), true)) {
				if (result == NULL || dpos.quick_length() < dist) {
					result = target;
					dist = dpos.quick_length();
				}
			}
		}
		
		if (result != NULL) {
			_state.fire = true;
			_direction = get_relative_position(result);
			_direction.normalize();
			set_direction(_direction.get_direction(get_directions_number()) - 1);
		}
	}
private: 
	const bool validateFire(const int idx) {
		if (idx == 0) 
			return canFire();
		return true;
	}
	
	Alarm _reaction; 
	bool _aim_missiles;
};

REGISTER_OBJECT("machinegunner", AITrooper, ("machinegunner-bullet", true));
REGISTER_OBJECT("thrower", AITrooper, ("thrower-missile", false));

REGISTER_OBJECT("machinegunner-in-watchtower", TrooperInWatchTower, ("machinegunner-bullet", true));
REGISTER_OBJECT("thrower-in-watchtower", TrooperInWatchTower, ("thrower-missile", false));
