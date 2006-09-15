#include <assert.h>
#include "resource_manager.h"
#include "object.h"
#include "world.h"
#include "game.h"
#include "tank.h"

REGISTER_OBJECT("tank", Tank, ());

Tank::Tank() 
: Object("player"), _fire(0.5, false) {
}

Tank::Tank(const std::string &animation) 
: Object("player"), _fire(0.5, false) {
	setup(animation);
}


void Tank::onSpawn() {
	Object *_smoke = spawnGrouped("single-pose", "tank-smoke", v3<float>(0,0,0.1), Centered);
	_smoke->impassability = 0;

	Object *_rockets = spawnGrouped("rockets-on-tank", "rockets-on-tank", v3<float>(0,0,0.1), Centered);
	_rockets->impassability = 0;

	add("rockets", _rockets);
	add("smoke", _smoke);
}

Object * Tank::clone() const {
	return new Tank(*this);
}

void Tank::emit(const std::string &event, BaseObject * emitter) {
	if (event == "death") {
		LOG_DEBUG(("dead"));
		cancelAll();
		//play("dead", true);
		spawn("corpse", "dead-" + animation, v3<float>(0,0,-0.5), v3<float>(0,0,0));
		_velocity.x = _velocity.y = _velocity.z = 0;
		Object::emit(event, emitter);
	} else if (event == "collision") {
		addDamage(emitter);
	} else if (event == "launch") {
		spawn("guided-rocket", "guided-rocket", v3<float>(0,0,1), _direction);
		const Object * la = ResourceManager.get_const()->getAnimation("rocket-launch");
		v3<float> dpos = (size - la->size).convert<float>();
		dpos.z = 1;
		dpos /= 2;

		Object *o = spawn("rocket-launch", "rocket-launch", dpos, _direction);
		o->setDirection(getDirection());
		//LOG_DEBUG(("dir: %d", o->getDirection()));else Object::emit(event, emitter);
	} else Object::emit(event, emitter);
}



void Tank::tick(const float dt) {
	Object::tick(dt);

	bool fire_possible = _fire.tick(dt);
	
	if (getState().empty()) {
		play("hold", true);
	}

	_velocity.normalize();
	if (_velocity.is0()) {
		cancelRepeatable();
		play("hold", true);
		groupEmit("rockets", "hold");
	} else {
		if (getState() == "hold") {
			cancelAll();
			play("start", false);
			play("move", true);
			groupEmit("rockets", "move");
		}
	}

	if (_state.fire && fire_possible) {
		_fire.reset();
		
		if (getState() == "fire") 
			cancel();
		
		playNow("fire");
		
		//LOG_DEBUG(("vel: %f %f", _state.old_vx, _state.old_vy));
		//v3<float> v = _velocity.is0()?_direction:_velocity;
		//v.normalize();
		spawn("bullet", "bullet", v3<float>(0,0,-0.1), _direction);
	}
	if (_state.alt_fire) {
		groupEmit("rockets", "launch");
	}
	
	_state.fire = false;
	
	limitRotation(dt, 8, 0.05, true, false);

	//LOG_DEBUG(("_velocity: %g %g", _velocity.x, _velocity.y));
}

