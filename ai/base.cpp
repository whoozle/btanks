
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
#include "base.h"
#include <assert.h>
#include "world.h"
#include "config.h"

using namespace ai;

Base::Base() : Object("player"), _reaction_time(true), _refresh_path(true), _target_id(-1) {}

Base::~Base() {
	LOG_DEBUG(("traits: \n%s", _traits.save().c_str()));
}

void Base::addEnemyClass(const std::string &classname) {
	_enemies.insert(classname);
}

void Base::addBonusName(const std::string &rname) {
	_bonuses.insert(rname);
}


void Base::onSpawn() {
	const std::string vehicle = getType();
	if (vehicle.empty())
		throw_ex(("vehicle MUST provide its type"));
	
	LOG_DEBUG(("spawning as '%s'", vehicle.c_str()));
	if (_enemies.size() == 0 && _bonuses.size()) 
		throw_ex(("vehicle was not provide enemies/bonuses"));
		
	float rt, rpi;
	Config->get("objects.ai-" + vehicle + ".reaction-time", rt, 0.1);
	_reaction_time.set(rt);
	Config->get("objects.ai-" + vehicle + ".refresh-path-interval", rpi, 1);
	_refresh_path.set(rpi);
	Config->get("objects.ai-" + vehicle + ".pathfinding-slice", _pf_slice, 10);
}

const bool Base::isEnemy(const Object *o) const {
	return _enemies.find(o->classname) != _enemies.end();
}


void Base::calculate(const float dt) {
	const bool refresh_path = _refresh_path.tick(dt);
	const bool dumb = !_reaction_time.tick(dt);
	const Object *target = NULL;
	
	if (!refresh_path && dumb) 
		goto gogogo;

	
	target = World->findTarget(this, _enemies, _bonuses, _traits);
	if (target != NULL && ((refresh_path && isEnemy(target)) || target->getID() != _target_id)) {
		_target_id = target->getID();
		_enemy = isEnemy(target);
				
		target->getPosition(_target_position);
		LOG_DEBUG(("next target: %s at %d,%d", target->registered_name.c_str(), _target_position.x, _target_position.y));
		findPath(_target_position, 16);
		//Way way;
		//if (!old_findPath(target, way))
		//	LOG_WARN(("no way"));
		//else setWay(way);

	}

	
	//bool driven = isDriven();
	
	//LOG_DEBUG(("calculating: %c, driven: %c", calculating?'+':'-', driven?'+':'-'));
	gogogo:
	
	Way way;
	
	if (calculatingPath()) {
		int n = 1;
		bool found;
		while(! (found = findPathDone(way)) && n < _pf_slice)
			++n;
		
		if (found) {
			//LOG_DEBUG(("n = %d", n));
			if (!way.empty()) {
				setWay(way);
			} else {
				LOG_WARN(("no path"));
			}
		}
	} else  {
		//LOG_DEBUG(("idle"));
		/*v3<float> dir = _target_position.convert<float>() - getPosition();
		dir.normalize();
		setDirection(dir.getDirection(getDirectionsNumber()) - 1);
		if (_enemy) 
			_state.fire = true;
		*/
	}
	
	calculateWayVelocity();
	if (!isDriven()) {
		//LOG_DEBUG(("stop!"));
		_velocity.clear();
	}
}


