#include <string.h>

#include "mrt/logger.h"

#include "keyplayer.h"
#include "game.h"
#include "resource_manager.h"
#include "animated_object.h"
#include "world.h"
#include "assert.h"

KeyPlayer::KeyPlayer(AnimatedObject *animation, SDLKey up, SDLKey down, SDLKey left, SDLKey right, SDLKey fire): 
_animation(animation),
_up(up), _down(down), _left(left), _right(right), _fire(fire) {
	speed = 300;
	_bullet = 0;
	//ttl = 1;
	stale = false;
	Game->key_signal.connect(sigc::mem_fun(this, &KeyPlayer::onKey));
	memset(&_state, 0, sizeof(_state));
	_animation->play("hold", true);
}

void KeyPlayer::tick(const float dt) {
	w = _animation->w;
	h = _animation->h;
	
	if (stale) 
		return;
	

	static int dirmap[] = {
		4, 3, 2,
		5, 0, 1,
		6, 7, 8,
	};
	
	v3<float> velocity;
	if (_state.left) velocity.x -= 1;
	if (_state.right) velocity.x += 1;
	if (_state.up) velocity.y -= 1;
	if (_state.down) velocity.y += 1;
	
	if (_velocity != velocity) {
		int dir = dirmap[(int)((velocity.y + 1) * 3 + velocity.x + 1)];
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
	_velocity = velocity;


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
		spawn(_bullet, v3<float>(), _old_velocity);
	}
	_state.fire = false;
	
	_animation->tick(dt);
}

void KeyPlayer::render(sdlx::Surface &surf, const int x, const int y) {
	_animation->render(surf, x, y);
}


KeyPlayer::~KeyPlayer() {}

void KeyPlayer::onKey(const Uint8 type, const SDL_keysym sym) {
	if (stale)
		return;
	
	bool *key = NULL;
	if (sym.sym == _up) {
		key = &_state.up;
	} else if (sym.sym == _down) {
		key = &_state.down;
	} else if (sym.sym == _left) {		
		key = &_state.left;
	} else if (sym.sym == _right) {		
		key = &_state.right;
	} else if (sym.sym == _fire && type == SDL_KEYDOWN) {
		key = &_state.fire;
	} else return;
	assert(key != NULL);
	
	if (type == SDL_KEYDOWN) {
		*key = true;
	} else if (type == SDL_KEYUP) {
		*key = false;
	}
}


void KeyPlayer::emit(const std::string &event, const Object * emitter) {
	if (event == "death") {
		LOG_DEBUG(("dead"));
		_animation->cancelAll();
		_animation->play("dead", true);
		stale = true;
		_velocity.x = _velocity.y = _velocity.z = 0;
	} else Object::emit(event, emitter);
}
