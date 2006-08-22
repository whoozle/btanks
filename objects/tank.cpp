#include <assert.h>
#include "resource_manager.h"
#include "object.h"
#include "world.h"
#include "game.h"
#include "tank.h"

REGISTER_OBJECT("tank", Tank, (false));

Tank::Tank(const bool stateless) 
: Object("player", stateless), _fire(0.5, false) {
}

Tank::Tank(const std::string &animation, const bool stateless) 
: Object("player", stateless), _fire(0.5, false) {
	setup(animation);
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


void Tank::emit(const std::string &event, const BaseObject * emitter) {
	if (event == "death") {
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
	bool fire_possible = _fire.tick(dt);
	bool notify = false;
	
	if (getState().empty()) {
		play("hold", true);
	}
	
	if (_velocity != _old_velocity) {
		int dir = v3<float>::getDirection8(_velocity);
		if (dir) {
			setDirection(dir - 1);
			//LOG_DEBUG(("animation state: %s", _animation->getState().c_str()));
			if (getState() == "hold") {
				cancelAll();
				play("start", false);
				play("move", true);
			}
			
//			_state.old_vx = _state.vx;
//			_state.old_vy = _state.vy;
		} else {
			cancelRepeatable();
			play("hold", true);
		}
		notify = true;
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
		notify = true;
	}
	if (notify) 
		Game->notify(_state);
	
	_state.fire = false;

	//LOG_DEBUG(("_velocity: %g %g", _velocity.x, _velocity.y));
	
	Object::tick(dt);
}

