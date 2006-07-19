#include "keyplayer.h"
#include "mrt/logger.h"
#include "game.h"
#include <string.h>

KeyPlayer::KeyPlayer(AnimatedObject *animation, SDLKey up, SDLKey down, SDLKey left, SDLKey right): 
_animation(animation),
_up(up), _down(down), _left(left), _right(right) {
	speed = 300;
	Game->key_signal.connect(sigc::mem_fun(this, &KeyPlayer::onKey));
	memset(&state, 0, sizeof(state));
	_animation->play("hold", true);
}

void KeyPlayer::tick(const float dt) {
	static int dirmap[] = {
		4, 3, 2,
		5, 0, 1,
		6, 7, 8,
	};
	if (_vx != state.vx || _vy != state.vy) {
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
		} else {
			_animation->cancelRepeatable();
			_animation->play("hold", true);
		}
	}
	_vx = state.vx;
	_vy = state.vy;
	_animation->tick(dt);
}

void KeyPlayer::render(sdlx::Surface &surf, const int x, const int y, int &w, int &h) {
	_animation->render(surf, x, y, w, h);
}


KeyPlayer::~KeyPlayer() {}

void KeyPlayer::onKey(const Uint8 type, const SDL_keysym sym) {
	float vx = 0, vy = 0;

	if (sym.sym == _up) {
		vy = -1;
	} else if (sym.sym == _down) {
		vy = 1;
	} else if (sym.sym == _left) {		
		vx = -1;
	} else if (sym.sym == _right) {		
		vx = 1;
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
