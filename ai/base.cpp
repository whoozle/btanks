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

void Base::calculate(const float dt) {
	if (!_reaction_time.tick(dt)) {
		calculateWayVelocity();
		return;
	}

	const Object *target = World->findTarget(this, _enemies, _bonuses, _traits);
	if (target != NULL && target->getID() != _target_id) {
		_target_id = target->getID();
		
		target->getPosition(_target_position);
		LOG_DEBUG(("next target: %s at %d,%d", target->registered_name.c_str(), _target_position.x, _target_position.y));
		findPath(_target_position, 24);
	}

	Way way;
	if (calculatingPath()) {
		if (findPathDone(way))
			setWay(way);
	} else {
		//_velocity = _target_position.convert<float>() - getPosition();
	}
	calculateWayVelocity();
}


