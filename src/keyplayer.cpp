#include "keyplayer.h"
#include "mrt/logger.h"
#include "game.h"
#include <string.h>

KeyPlayer::KeyPlayer(AnimatedObject *animation, SDLKey up, SDLKey down, SDLKey left, SDLKey right): 
_animation(animation),
_up(up), _down(down), _left(left), _right(right) {
	Game->key_signal.connect(sigc::mem_fun(this, &KeyPlayer::onKey));
	memset(&state, 0, sizeof(state));
}

void KeyPlayer::tick(const float dt) {
	_vx = state.vx;
	_vy = state.vy;
	_animation->tick(dt);
}

void KeyPlayer::render(sdlx::Surface &surf, const int x, const int y) {
	_animation->render(surf, x, y);
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
}
