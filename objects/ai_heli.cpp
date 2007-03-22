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
		_reaction.serialize(s);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Heli::deserialize(s);
		_reaction.deserialize(s);
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
	v2<int> size = Map->getSize();
	
	for(int i = 0; i < 3; ++i) {
		v2<int> next_target;
		next_target.x = mrt::random(size.x);
		next_target.y = mrt::random(size.y);
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
		
	_state.fire = true;
	

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
}

REGISTER_OBJECT("helicopter", AIHeli, ());
