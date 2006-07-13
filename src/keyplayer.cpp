#include "keyplayer.h"
#include "mrt/logger.h"
#include "game.h"
#include <string.h>

KeyPlayer::KeyPlayer() {
	Game->key_signal.connect(sigc::mem_fun(this, &KeyPlayer::onKey));
	memset(&state, 0, sizeof(state));
}

void KeyPlayer::tick(const float dt) {
	setV(state.vx, state.vy);
}


KeyPlayer::~KeyPlayer() {}

void KeyPlayer::onKey(const Uint8 type, const SDL_keysym sym) {
	float vx = 0, vy = 0;
	switch(sym.sym) {
	case SDLK_UP: 
		vy = -1;
		break;
	case SDLK_DOWN: 
		vy = 1;
		break;
	case SDLK_LEFT: 
		vx = -1;
		break;
	case SDLK_RIGHT: 
		vx = 1;
		break;
	default:
		break;
	}

	if (type == SDL_KEYDOWN) {
		state.vx += vx;
		state.vy += vy;
	} else if (type == SDL_KEYUP) {
		state.vx -= vx;
		state.vy -= vy;
	}
}
