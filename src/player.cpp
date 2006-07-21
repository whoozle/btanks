#include "player.h"
#include "resource_manager.h"
#include "animated_object.h"
#include "world.h"
#include <assert.h>

Player::Player(const std::string &classname, const std::string &animation, const bool stateless) 
: AnimatedObject(classname), _stale(false), _stateless(stateless) {
	ResourceManager->initMe(this, animation);
	
	LOG_DEBUG(("player %p: %s", (void *)this, classname.c_str()));
	
	speed = 300;
	_bullet = 0;
	memset(&_state, 0, sizeof(_state));
	//ttl = 1;

	//_animation = ResourceManager->createAnimation(animation);
	play("hold", true);
}

void Player::emit(const std::string &event, const Object * emitter) {
	if (event == "death") {
		LOG_DEBUG(("dead"));
		cancelAll();
		play("dead", true);
		_stale = true;
		_velocity.x = _velocity.y = _velocity.z = 0;
	} else Object::emit(event, emitter);
}



void Player::tick(const float dt) {
	if (_stale) 
		return;
	
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
		//LOG_DEBUG(("pose %d", pose));
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
	}

	if (_state.fire && !World->exists(_bullet)) {
		if (getState() == "fire") 
			cancel();
		
		playNow("fire");
		
		//LOG_DEBUG(("vel: %f %f", _state.old_vx, _state.old_vy));
		v3<float> v = _velocity.is0()?_direction:_velocity;
		_bullet = spawn("bullet", v * 20, v);
	}
	_state.fire = false;
	
	AnimatedObject::tick(dt);
}
