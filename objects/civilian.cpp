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

#include "trooper.h"
#include "resource_manager.h"
#include "ai/waypoints.h"

class Civilian : public Trooper {
public: 
	Civilian(const std::string &classname) : Trooper(classname, std::string()) {} 
};

class AICivilian : public Civilian, public ai::Waypoints  {
public: 
	AICivilian() : Civilian("civilian"), _thinking_timer(true), _guard_timer(false), _thinking(false), _guard(false) {}
	void onSpawn() {
		//GET_CONFIG_VALUE("object.civilian.thinking-duration", float, td, 3.0f);
		_thinking_timer.set(3.0f);
		_guard_timer.set(2.0f);
	
		_pose = "walk";
		disown();

		Trooper::onSpawn();

		_avoid_obstacles = true;
		_stop_on_obstacle = false;
		
		ai::Waypoints::onSpawn(this);
	}
	void tick(const float dt) {
		if (_thinking) {
			if (getState() != "thinking") {
				cancelAll();
				play("thinking", true);
				LOG_DEBUG(("playing thinking..."));
			}
		} else Trooper::tick(dt);
	}
	
	void calculate(const float dt) {
		if (_thinking_timer.tick(dt)) { 
			_thinking = false;
			_guard_timer.reset();
			_guard = true;
			LOG_DEBUG(("stop thinking, guard interval signalled"));
		}
		
		if (_guard_timer.tick(dt))
			_guard = false;
		
		if (_thinking) {
			_velocity.clear();
		} else {
			ai::Waypoints::calculate(this, dt);
			if (_guard) {
				_velocity.normalize();
				int dir = getDirection(), dirs = getDirectionsNumber();
				if (dir >= 0) {
					v2<float> dv; 
					dv.fromDirection((dir - 1 + dirs) % dirs, dirs);
					_velocity += dv / 2;
				}
			}
		}
		updateStateFromVelocity();
	}

	virtual void onObstacle(const Object *o) {
		if (_guard)
			return;
		LOG_DEBUG(("%d:%s: obstacle %s", getID(), animation.c_str(), o->animation.c_str()));
		_thinking = true;
		_thinking_timer.reset();

		int dirs = getDirectionsNumber();
		setDirection(getRelativePosition(o).getDirection(dirs));
	}

	virtual void serialize(mrt::Serializator &s) const {
		Civilian::serialize(s);
		s.add(_thinking_timer);
		s.add(_guard_timer);
		s.add(_thinking);
		s.add(_guard);
	}

	virtual void deserialize(const mrt::Serializator &s) {
		Civilian::deserialize(s);
		s.get(_thinking_timer);
		s.get(_guard_timer);
		s.get(_thinking);
		s.get(_guard);
	}

	Object *clone() const { return new AICivilian(*this); }
private: 
	Alarm _thinking_timer, _guard_timer;
	bool _thinking, _guard;
};

REGISTER_OBJECT("civilian", AICivilian, ());
