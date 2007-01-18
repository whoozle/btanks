
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

Base::Base() : Object("player"), _reaction_time(true), _target_id(-1) {}

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
		
	float rt;
	Config->get("objects.ai-" + vehicle + ".reaction-time", rt, 0.1);
	_reaction_time.set(rt);
}

const bool Base::isEnemy(const Object *o) const {
	return _enemies.find(o->classname) != _enemies.end();
}


void Base::calculate(const float dt) {
	if (!_reaction_time.tick(dt)) {
		calculateWayVelocity();
		return;
	}

	const Object *target = World->findTarget(this, _enemies, _bonuses, _traits);
	if (target != NULL && target->getID() != _target_id) {
		_target_id = target->getID();
		_enemy = isEnemy(target);
				
		target->getPosition(_target_position);
		LOG_DEBUG(("next target: %s at %d,%d", target->registered_name.c_str(), _target_position.x, _target_position.y));
		findPath(_target_position, 16);
/*		Way way;
		if (!World->old_findPath(this, _target_position.convert<float>() - getPosition(), way, target))
			LOG_WARN(("no way"));
		else setWay(way);
*/
	}


	Way way;
	
	bool calculating = calculatingPath();
	bool driven = isDriven();
	
	LOG_DEBUG(("calculating: %c, driven: %c", calculating?'+':'-', driven?'+':'-'));
	
	if (calculating) {
		int n = 0;
		while(!findPathDone(way)) ++n;
		LOG_DEBUG(("n = %d", n));
		if (way.empty()) 
			LOG_WARN(("no path"));
		//if (findPathDone(way))
			setWay(way);
	} else {
		v3<float> dir = _target_position.convert<float>() - getPosition();
		dir.normalize();
		setDirection(dir.getDirection(getDirectionsNumber()) - 1);
		if (_enemy) 
			_state.fire = true;
	}
	
	calculateWayVelocity();
}


