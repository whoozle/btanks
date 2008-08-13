
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
#include "heli.h"
#include "config.h"
#include "registrar.h"
#include "tmx/map.h"
#include "mrt/random.h"
#include "ai/base.h"
#include "ai/targets.h"

class AIHeli : public Heli, public ai::Base {
public:
	AIHeli() : Heli("helicopter"), _reaction(true), _target_dir(-1), _moving_time(0) {}
	
	virtual void on_spawn();
	void calculate(const float dt);
	virtual void serialize(mrt::Serializator &s) const {
		Heli::serialize(s);
		ai::Base::serialize(s);
		s.add(_reaction);
		s.add(_moving_time);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Heli::deserialize(s);
		ai::Base::deserialize(s);
		s.get(_reaction);
		s.get(_moving_time);
	}
	
	virtual const bool validateFire(const int idx) {
		return (idx == 0)? canFire() : true;
	}

	virtual Object * clone() const { return new AIHeli(*this); }
	virtual void onIdle(const float dt);
	
private: 
	Alarm _reaction;
	int _target_dir;
	float _moving_time;
};

void AIHeli::onIdle(const float dt) {
	Way way;
	v2<int> map_size = Map->get_size();
	
	for(int i = 0; i < 2; ++i) {
		v2<int> next_target;
		next_target.x = (int)size.x / 2 + mrt::random(map_size.x - (int)size.x);
		next_target.y = (int)size.y / 2 + mrt::random(map_size.y - (int)size.y);
		way.push_back(next_target);		
	}
	set_way(way);
}


void AIHeli::on_spawn() {
	GET_CONFIG_VALUE("objects.helicopter.reaction-time", float, rt, 0.1);
	mrt::randomize(rt, rt/10);
	_reaction.set(rt);
	Heli::on_spawn();
	ai::Base::on_spawn(this);
	ai::Base::multiplier = 3.0f;
}

void AIHeli::calculate(const float dt) {
	v2<float> vel;
	if (!_reaction.tick(dt))
		goto done;
		
	_state.fire = false;
	
	_target_dir = get_target_position(_velocity, ai::Targets->troops, "helicopter-bullet");
	if (_target_dir >= 0) {
		//LOG_DEBUG(("target: %g %g %g, dir: %d", _velocity.x, _velocity.y, _velocity.length(), _target_dir));
		/*
		Way way;
		if (find_path(tp, way)) {
		set_way(way);
			calculate_way_velocity();
		}
		*/
		if (_velocity.length() >= 25) {
			quantize_velocity();
			//_direction.fromDirection(get_direction(), get_directions_number());
		} else {
			_velocity.clear();
			set_direction(_target_dir);
			//LOG_DEBUG(("%d", _target_dir));
			_direction.fromDirection(_target_dir, get_directions_number());
		}

		if (_target_dir == get_direction()) {
			_state.fire = true;
		}	
	} 
	
	if (_target_dir < 0 && !is_driven()) {
		_velocity.clear();
		_target_dir = -1;
		onIdle(dt);
	}
	
done: 	
	GET_CONFIG_VALUE("engine.mass-acceleration-divisor", float, ac_div, 1000.0f);

	const float ac_t = mass / ac_div * 0.8;
	_state.alt_fire = _moving_time >= ac_t;

	calculate_way_velocity();
	
	if (!_velocity.is0()) {
		_moving_time += dt;
	} else 
		_moving_time = 0;

	GET_CONFIG_VALUE("objects.helicopter.rotation-time", float, rt, 0.2f);
	limit_rotation(dt, rt, true, true);	
	update_state_from_velocity();
}

REGISTER_OBJECT("helicopter", AIHeli, ());
