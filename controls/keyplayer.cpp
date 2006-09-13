#include <string.h>

#include "mrt/logger.h"

#include "keyplayer.h"
#include "object.h"
#include "world.h"
#include <assert.h>

KeyPlayer::KeyPlayer(SDLKey up, SDLKey down, SDLKey left, SDLKey right, SDLKey fire, SDLKey alt_fire): 
	_up(up), _down(down), _left(left), _right(right), _fire(fire), _alt_fire(alt_fire) {
}

void KeyPlayer::updateState(PlayerState &state) {
	static const Uint8 *keys = SDL_GetKeyState(0);
	state.left = keys[_left] != 0;
	state.right = keys[_right] != 0;
	state.up = keys[_up] != 0;
	state.down = keys[_down] != 0;
	state.fire = keys[_fire] != 0;
	state.alt_fire = keys[_alt_fire] != 0;
}
