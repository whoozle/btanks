#include <assert.h>
#include "resource_manager.h"
#include "object.h"
#include "world.h"
#include "game.h"
#include "tank.h"

REGISTER_OBJECT("tank", Tank, ());

Tank::Tank() 
: Object("player"), _fire(0.5, false), _smoke(NULL) {
}

Tank::Tank(const std::string &animation) 
: Object("player"), _fire(0.5, false), _smoke(NULL) {
	setup(animation);
}


void Tank::onSpawn() {
	_smoke = spawnGrouped("single-pose", "tank-smoke", v3<float>(0,0,0.1), Centered);
	_smoke->hp = 100000;
	_smoke->impassability = 0;
}

Object * Tank::clone(const std::string &opt) const {
	Tank *p = NULL;
	TRY { 
		//LOG_DEBUG(("cloning player with animation '%s'", opt.c_str()));
		p = new Tank(*this);
		p->setup(opt);
	} CATCH("clone", { delete p; throw; });
	return p;
}


void Tank::emit(const std::string &event, BaseObject * emitter) {
	if (event == "death") {
		_smoke->emit(event, emitter);
		LOG_DEBUG(("dead"));
		cancelAll();
		//play("dead", true);
		spawn("corpse", "dead-" + animation, v3<float>(0,0,-0.5), v3<float>(0,0,0));
		_velocity.x = _velocity.y = _velocity.z = 0;
		Object::emit(event, emitter);
	} else if (event == "collision") {
		const std::string &c = emitter->classname;
		if (c == "bullet") {
			spawn("explosion", "explosion", v3<float>(0,0,1), v3<float>(0,0,0));
			hp -= emitter->hp;	
			LOG_DEBUG(("received %d hp of damage. hp = %d", emitter->hp, hp));
			if (hp <= 0) 
				emit("death", emitter);
		}
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
	} else {
		int dir = _velocity.getDirection8();
		if (dir) {
			setDirection(dir - 1);
			//LOG_DEBUG(("animation state: %s", _animation->getState().c_str()));
			if (getState() == "hold") {
				cancelAll();
				play("start", false);
				play("move", true);
			}
		}
	}

	if (_state.fire && fire_possible) {
		_fire.reset();
		
		if (getState() == "fire") 
			cancel();
		
		playNow("fire");
		
		//LOG_DEBUG(("vel: %f %f", _state.old_vx, _state.old_vy));
		v3<float> v = _velocity.is0()?_direction:_velocity;
		v.normalize();
		spawn("bullet", "bullet", v3<float>(0,0,-0.1), v);
	}
	
	_state.fire = false;

	//LOG_DEBUG(("_velocity: %g %g", _velocity.x, _velocity.y));
}

