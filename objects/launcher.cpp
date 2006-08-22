#include "resource_manager.h"
#include "object.h"
#include "world.h"
#include "game.h"
#include "launcher.h"

REGISTER_OBJECT("launcher", Launcher, (false));

Launcher::Launcher(const bool stateless) 
: Object("player", stateless), _fire(0.3, false), _smoke(0), _rockets(0) {
}

Launcher::Launcher(const std::string &animation, const bool stateless) 
: Object("player", stateless), _fire(0.3, false), _smoke(0), _rockets(0) {
	setup(animation);
}

Object * Launcher::clone(const std::string &opt) const {
	Launcher *p = NULL;
	TRY { 
		//LOG_DEBUG(("cloning player with animation '%s'", opt.c_str()));
		p = new Launcher(*this);
		p->setup(opt);
	} CATCH("clone", { delete p; throw; });
	return p;
}

void Launcher::onSpawn() {
	_smoke = spawnGrouped("single-pose", "smoke", v3<float>(0,0,-0.1), Centered);
	_rockets = spawnGrouped("rockets-in-vehicle", "rockets-in-vehicle", v3<float>(0,0,0.1), Centered);
}


void Launcher::emit(const std::string &event, const BaseObject * emitter) {
	if (event == "death") {
		_smoke->emit(event, this);
		_rockets->emit(event, this);
		_smoke = _rockets = NULL;
		LOG_DEBUG(("dead"));
		cancelAll();
		//play("dead", true);
/*		spawn("corpse", "dead-" + animation, v3<float>(0,0,-0.5), v3<float>(0,0,0));
		_velocity.x = _velocity.y = _velocity.z = 0;
*/
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



void Launcher::tick(const float dt) {
	bool fire_possible = _fire.tick(dt);
	bool notify = false;
	
	if (getState().empty()) {
		play("hold", true);
		_rockets->emit("hold", this);
	}
	
	if (_velocity != _old_velocity) {
		int dir = v3<float>::getDirection8(_velocity);
		if (dir) {
			setDirection(dir - 1);
			//LOG_DEBUG(("animation state: %s", _animation->getState().c_str()));
			if (getState() == "hold") {
				cancelAll();
				//play("start", false);
				play("move", true);
				_rockets->emit("move", this);
			}
			
//			_state.old_vx = _state.vx;
//			_state.old_vy = _state.vy;
		} else {
			cancelRepeatable();
			play("hold", true);
			_rockets->emit("hold", this);
		}
		notify = true;
	}

	if (_state.fire && fire_possible) {
		_fire.reset();
		_rockets->emit("launch", this);
		notify = true;
	}
	if (notify) 
		Game->notify(_state);
	
	_state.fire = false;

	//LOG_DEBUG(("_velocity: %g %g", _velocity.x, _velocity.y));
	
	Object::tick(dt);
}

