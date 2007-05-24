
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
#include "heli.h"
#include "config.h"
#include "resource_manager.h"
#include "player_manager.h"
#include "tmx/map.h"
#include "mrt/random.h"

class AIHeli : public Heli {
public:
	AIHeli() : Heli("helicopter"), _reaction(true), _target_dir(-1) {
			_targets.insert("missile");	
			_targets.insert("player");
			_targets.insert("trooper");
			_targets.insert("kamikaze");
			_targets.insert("boat");		
			_targets.insert("helicopter");
	}
	virtual void onSpawn();
	void calculate(const float dt);
	virtual void serialize(mrt::Serializator &s) const {
		Heli::serialize(s);
		s.add(_reaction);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Heli::deserialize(s);
		s.get(_reaction);
	}

	virtual Object * clone() const { return new AIHeli(*this); }
	virtual void onIdle(const float dt);
	
private: 
	Alarm _reaction;	
	int _target_dir;
	
	std::set<std::string> _targets;
};

void AIHeli::onIdle(const float dt) {
	if (PlayerManager->isClient())
		return;

	Way way;
	v2<int> map_size = Map->getSize();
	
	for(int i = 0; i < 3; ++i) {
		v2<int> next_target;
		next_target.x = (int)size.x / 2 + mrt::random(map_size.x - (int)size.x);
		next_target.y = (int)size.y / 2 + mrt::random(map_size.y - (int)size.y);
		way.push_back(next_target);		
	}
	setWay(way);
}


void AIHeli::onSpawn() {
	GET_CONFIG_VALUE("objects.helicopter.reaction-time", float, rt, 0.1);
	mrt::randomize(rt, rt/10);
	_reaction.set(rt);
	Heli::onSpawn();
}

void AIHeli::calculate(const float dt) {
	v2<float> vel;
	if (!_reaction.tick(dt) || isDriven())
		goto done;
		
	_state.fire = false;
	
	_target_dir = getTargetPosition(_velocity, _targets, "helicopter-bullet");
	if (_target_dir >= 0) {
		//LOG_DEBUG(("target: %g %g %g", tp.x, tp.y, tp.length()));
		/*
		Way way;
		if (findPath(tp, way)) {
		setWay(way);
			calculateWayVelocity();
		}
		*/
		_state.fire = true;
		if (_velocity.length() >= 9) {
			quantizeVelocity();
			_direction.fromDirection(getDirection(), getDirectionsNumber());
		} else {
			_velocity.clear();
			setDirection(_target_dir);
			//LOG_DEBUG(("%d", _target_dir));
			_direction.fromDirection(_target_dir, getDirectionsNumber());
		}
	
	} else {
		_velocity.clear();
		_target_dir = -1;
		onIdle(dt);
	}
	
done: 	
	GET_CONFIG_VALUE("engine.mass-acceleration-divisor", float, ac_div, 1000.0);

	const float ac_t = mass / ac_div * 0.8;
	_state.alt_fire = _moving_time >= ac_t;

	calculateWayVelocity();
	updateStateFromVelocity();

	GET_CONFIG_VALUE("objects.helicopter.rotation-time", float, rt, 0.2);
	limitRotation(dt, rt, true, true);	
	updateStateFromVelocity();
}

REGISTER_OBJECT("helicopter", AIHeli, ());
