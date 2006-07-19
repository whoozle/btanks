#include "keyplayer.h"
#include "mrt/logger.h"
#include "game.h"
#include <string.h>
#include "resource_manager.h"
#include "world.h"

KeyPlayer::KeyPlayer(AnimatedObject *animation, SDLKey up, SDLKey down, SDLKey left, SDLKey right, SDLKey fire): 
_animation(animation),
_up(up), _down(down), _left(left), _right(right), _fire(fire) {
	speed = 300;
	_bullet = 0;
	//ttl = 1;
	stale = false;
	Game->key_signal.connect(sigc::mem_fun(this, &KeyPlayer::onKey));
	memset(&state, 0, sizeof(state));
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
	if (_velocity.x != state.vx || _velocity.y != state.vy) {
		int dir = dirmap[(int)((state.vy + 1) * 3 + state.vx + 1)];
		//LOG_DEBUG(("pose %d", pose));
		if (dir) {
			_animation->setDirection(dir - 1);
			//LOG_DEBUG(("animation state: %s", _animation->getState().c_str()));
			if (_animation->getState() == "hold") {
				_animation->cancelAll();
				_animation->play("start", false);
				_animation->play("move", true);
			}
			
			state.old_vx = state.vx;
			state.old_vy = state.vy;
		} else {
			_animation->cancelRepeatable();
			_animation->play("hold", true);
		}
	}
	_velocity.x = state.vx;
	_velocity.y = state.vy;


	if (state.fire && !World->exists(_bullet)) {
		if (_animation->getState() == "fire") 
			_animation->cancel();
		
		_animation->playNow("fire");
		
		_bullet = ResourceManager->createAnimation("bullet");
		_bullet->speed = 500;
		_bullet->ttl = 1;
		_bullet->piercing = true;
		
		_bullet->play("move", true);
		_bullet->setDirection(_animation->getDirection());
		//LOG_DEBUG(("vel: %f %f", state.old_vx, state.old_vy));
		spawn(_bullet, v3<float>(), v3<float>(state.old_vx, state.old_vy, 0));
	}
	state.fire = false;
	
	_animation->tick(dt);
}

void KeyPlayer::render(sdlx::Surface &surf, const int x, const int y) {
	_animation->render(surf, x, y);
}


KeyPlayer::~KeyPlayer() {}

void KeyPlayer::onKey(const Uint8 type, const SDL_keysym sym) {
	if (stale)
		return;
	
	float vx = 0, vy = 0;

	if (sym.sym == _up) {
		vy = -1;
	} else if (sym.sym == _down) {
		vy = 1;
	} else if (sym.sym == _left) {		
		vx = -1;
	} else if (sym.sym == _right) {		
		vx = 1;
	} else if (sym.sym == _fire && type == SDL_KEYDOWN) {
		state.fire = true;
	} else return;

	if (type == SDL_KEYDOWN) {
		state.vx += vx;
		state.vy += vy;
	} else if (type == SDL_KEYUP) {
		state.vx -= vx;
		state.vy -= vy;
	}
	//LOG_DEBUG(("%f, %f", state.vx, state.vy));
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
