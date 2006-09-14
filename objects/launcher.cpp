#include "resource_manager.h"
#include "object.h"
#include "world.h"
#include "game.h"
#include "launcher.h"

REGISTER_OBJECT("launcher", Launcher, ());

Launcher::Launcher() 
: Object("player"), _fire(0.3, false) {
}

Launcher::Launcher(const std::string &animation) 
: Object("player"), _fire(0.3, false) {
	setup(animation);
}

Object * Launcher::clone() const {
	return new Launcher(*this);
}

void Launcher::onSpawn() {
	Object *_smoke = spawnGrouped("single-pose", "launcher-smoke", v3<float>(0,0,0.1), Centered);
	_smoke->hp = 100000;
	_smoke->impassability = 0;
	add("smoke", _smoke);
	Object *_rockets = spawnGrouped("rockets-on-launcher", "rockets-on-launcher", v3<float>(0,0,0.1), Centered);
	_rockets->hp = 100000;
	_rockets->impassability = 0;
	add("rockets", _rockets);
}


void Launcher::emit(const std::string &event, BaseObject * emitter) {
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
		v3<float> v = _velocity.is0()?_direction:_velocity;
		v.normalize();
		spawn("rocket", "rocket", v3<float>(0,0,1), v);
		const Object * la = ResourceManager.get_const()->getAnimation("rocket-launch");
		v3<float> dpos = (size - la->size).convert<float>();
		dpos.z = 1;
		dpos /= 2;

		Object *o = spawn("rocket-launch", "rocket-launch", dpos, _direction);
		o->setDirection(getDirection());
		//LOG_DEBUG(("dir: %d", o->getDirection()));
	} else Object::emit(event, emitter);
}



void Launcher::tick(const float dt) {
	Object::tick(dt);
	bool fire_possible = _fire.tick(dt);
	
	if (getState().empty()) {
		play("hold", true);
		groupEmit("rockets", "hold");
	}

	if (_velocity.is0()) {	
		cancelRepeatable();
		play("hold", true);
		groupEmit("rockets", "hold");
	} else {
		if (getState() == "hold") {
			cancelAll();
			//play("start", false);
			play("move", true);
			groupEmit("rockets", "move");
		}
	}

	if (_state.fire && fire_possible) {
		_fire.reset();
		groupEmit("rockets", "launch");
	}

	_state.fire = false;
	limitRotation(dt, 8, 0.2, false, true);
	//LOG_DEBUG(("_velocity: %g %g", _velocity.x, _velocity.y));
}

