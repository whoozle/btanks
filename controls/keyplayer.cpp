#include <string.h>

#include "mrt/logger.h"

#include "keyplayer.h"
#include "game.h"
#include "object.h"
#include "world.h"
#include "assert.h"

KeyPlayer::KeyPlayer(SDLKey up, SDLKey down, SDLKey left, SDLKey right, SDLKey fire): 
	_up(up), _down(down), _left(left), _right(right), _fire(fire) {
	Game->key_signal.connect(sigc::mem_fun(this, &KeyPlayer::onKey));
}

void KeyPlayer::onKey(const Uint8 type, const SDL_keysym sym) {
	bool *key = NULL;
	if (sym.sym == _up) {
		key = &_state.up;
	} else if (sym.sym == _down) {
		key = &_state.down;
	} else if (sym.sym == _left) {		
		key = &_state.left;
	} else if (sym.sym == _right) {		
		key = &_state.right;
	} else if (sym.sym == _fire) {
		key = &_state.fire;
	} else return;
	assert(key != NULL);
	
	if (type == SDL_KEYDOWN) {
		*key = true;
	} else if (type == SDL_KEYUP) {
		*key = false;
	}
}


void KeyPlayer::updateState(PlayerState &state) {
	state = _state;
}
