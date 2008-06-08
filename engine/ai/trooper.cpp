#include "ai/trooper.h"
#include "config.h"
#include "mrt/random.h"
#include "mrt/serializator.h"
#include "object.h"

using namespace ai;

StupidTrooper::StupidTrooper(const std::string &object, const std::set<std::string> &targets) : 
	_object(object), _reaction(true), _target_dir(-1), _targets(targets) {}

void StupidTrooper::onSpawn() {
	GET_CONFIG_VALUE("objects.ai-trooper.reaction-time", float, rt, 0.15f);
	mrt::randomize(rt, rt / 10);
	//LOG_DEBUG(("rt = %g", rt));
	_reaction.set(rt);	
}

void StupidTrooper::serialize(mrt::Serializator &s) const {
	s.add(_object);
	s.add(_reaction);
	s.add(_target_dir);
}

void StupidTrooper::deserialize(const mrt::Serializator &s) {
	s.get(_object);
	s.get(_reaction);
	s.get(_target_dir);
}
	
StupidTrooper::~StupidTrooper() {}

void StupidTrooper::calculate(Object *object, PlayerState &_state, v2<float> &_velocity, v2<float> &_direction, const float dt) {
	int dirs = object->get_directions_number();
	if (!_reaction.tick(dt)) {
		return;
	}
	
	float range = object->getWeaponRange(_object);

	_target_dir = object->getTarget_position(_velocity, _targets, range);
	if (_target_dir >= 0) {
		//LOG_DEBUG(("target: %g %g %g", tp.x, tp.y, tp.length()));
		/*
		Way way;
		if (findPath(tp, way)) {
		setWay(way);
			calculateWayVelocity();
		}
		*/
		if (_velocity.length() >= 9) {
			object->quantizeVelocity();
			_direction.fromDirection(object->get_direction(), dirs);
			_state.fire = false;
		} else {
			_velocity.clear();
			object->setDirection(_target_dir);
			//LOG_DEBUG(("%d", _target_dir));
			_direction.fromDirection(_target_dir, dirs);
			_state.fire = true;
		}
	
	} else {
		_velocity.clear();
		_target_dir = -1;
		onIdle();
		_state.fire = false;
	}
}
