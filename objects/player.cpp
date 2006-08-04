#include <assert.h>
#include "player.h"
#include "resource_manager.h"
#include "object.h"
#include "world.h"
#include "game.h"

REGISTER_OBJECT("player", Player, (true));

Player::Player(const bool stateless) 
: Object("player"), _stale(false), _stateless(stateless), _fire(0.5, false) {}

void Player::setup(const std::string &animation) {
	_animation = animation;
	ResourceManager->initMe(this, animation);
	
	LOG_DEBUG(("player %p: %s", (void *)this, classname.c_str()));
	
	speed = 300;
	
	memset(&_state, 0, sizeof(_state));
	hp = 5;
	//ttl = 1;

	//_animation = ResourceManager->createAnimation(animation);
	play("hold", true);
}


Player::Player(const std::string &animation, const bool stateless) 
: Object("player"), _stale(false), _stateless(stateless), _fire(0.5, false), _animation(animation) {
	setup(animation);
}

Object * Player::clone(const std::string &opt) const {
	Player *p = NULL;
	TRY { 
		//LOG_DEBUG(("cloning player with animation '%s'", opt.c_str()));
		p = new Player(*this);
		p->setup(opt);
	} CATCH("clone", { delete p; throw; });
	return p;
}


void Player::emit(const std::string &event, const BaseObject * emitter) {
	if (_stale)
		return;
	
	if (event == "death") {
		LOG_DEBUG(("dead"));
		cancelAll();
		//play("dead", true);
		spawn("corpse", "dead-" + _animation, v3<float>(0,0,-0.5), v3<float>(0,0,0));
		_stale = true;
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



void Player::tick(const float dt) {
	if (_stale) {
		_velocity.clear();
		return;
	}
	bool fire_possible = _fire.tick(dt);
	bool notify = false;
	//AI player will be easier to implement if operating directly with velocity
	
	if (_stateless) {
		v3<float>::quantize(_velocity.x);	
		v3<float>::quantize(_velocity.y);
		
		_state.left = _velocity.x == -1;
		_state.right = _velocity.x == 1;
		_state.up = _velocity.y == -1;
		_state.down = _velocity.y == 1;
	}
	
	_velocity.clear();
	
	if (_state.left) _velocity.x -= 1;
	if (_state.right) _velocity.x += 1;
	if (_state.up) _velocity.y -= 1;
	if (_state.down) _velocity.y += 1;
	
	_velocity.normalize();
	
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
	
	Object::tick(dt);
}
