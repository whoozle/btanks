#include "player.h"
#include "resource_manager.h"
#include "animated_object.h"
#include "world.h"
#include <assert.h>

Player::Player(const std::string &classname, const std::string &animation, const bool stateless) 
: Object(classname), _stale(false), _stateless(stateless) {
	LOG_DEBUG(("player %p: %s", (void *)this, classname.c_str()));
	
	speed = 300;
	_bullet = 0;
	memset(&_state, 0, sizeof(_state));
	//ttl = 1;

	_animation = ResourceManager->createAnimation(animation);
	_animation->play("hold", true);
}

void Player::emit(const std::string &event, const Object * emitter) {
	if (event == "death") {
		LOG_DEBUG(("dead"));
		_animation->cancelAll();
		_animation->play("dead", true);
		_stale = true;
		_velocity.x = _velocity.y = _velocity.z = 0;
	} else Object::emit(event, emitter);
}


static inline int c2d(const float c) {
	if (c > 0.9238795325112867385) //cos(22.5)
		return 0;
	else if (c > 0.3826834323650898373) //cos(67.5)
		return 1;
	else if (c > -0.3826834323650898373)
		return 2;
	else if (c > -0.9238795325112867385)
		return 3;
	return 4;
}

static int v3dir(const v3<float> &v) {
	if (v.is0())
		return 0;

	int x = c2d(v.x) + 1;
	if (v.y <= 0) 
		return x;
	else {
		//LOG_DEBUG(("%d %d", x, 10 - x));
		return 10 - x;
	}
	return 0;
}

static void quantize(float &x) {
	if (x > 0.3826834323650898373) {
		x = 1;
	} else if (x < -0.3826834323650898373)
		x = -1;
	else x = 0;
}

void Player::tick(const float dt) {
	size = _animation->size;
	
	if (_stale) 
		return;
	
	//AI player will be easier to implement if operating directly with velocity
	
	if (_stateless) {
		quantize(_velocity.x);	
		quantize(_velocity.y);
		
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
		int dir = v3dir(_velocity);
		//LOG_DEBUG(("pose %d", pose));
		if (dir) {
			_animation->setDirection(dir - 1);
			//LOG_DEBUG(("animation state: %s", _animation->getState().c_str()));
			if (_animation->getState() == "hold") {
				_animation->cancelAll();
				_animation->play("start", false);
				_animation->play("move", true);
			}
			
//			_state.old_vx = _state.vx;
//			_state.old_vy = _state.vy;
		} else {
			_animation->cancelRepeatable();
			_animation->play("hold", true);
		}
	}

	if (_state.fire && !World->exists(_bullet)) {
		if (_animation->getState() == "fire") 
			_animation->cancel();
		
		_animation->playNow("fire");
		
		_bullet = ResourceManager->createAnimation("bullet");
		_bullet->speed = 500;
		_bullet->ttl = 1;
		_bullet->piercing = true;
		
		_bullet->play("move", true);
		_bullet->setDirection(_animation->getDirection());
		//LOG_DEBUG(("vel: %f %f", _state.old_vx, _state.old_vy));
		v3<float> v = _velocity.is0()?_direction:_velocity;
		spawn(_bullet, v * 20, v);
	}
	_state.fire = false;
	
	_animation->tick(dt);
}

void Player::render(sdlx::Surface &surf, const int x, const int y) {
	_animation->render(surf, x, y);
}

void Player::setDirection(const int d) {
	_animation->setDirection(d);
}
