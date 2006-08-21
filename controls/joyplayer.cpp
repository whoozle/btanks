#include <SDL/SDL.h>
#include "joyplayer.h"
#include "player_state.h"
#include "mrt/logger.h"

JoyPlayer::JoyPlayer(const int idx, const int fire)
: _fire(fire) {
	_joy.open(idx);
}

#define THRESHOLD 16384

void JoyPlayer::updateState(PlayerState &_state) {
	SDL_JoystickUpdate();
	Sint16 x = _joy.getAxis(0);
	Sint16 y = _joy.getAxis(1);
	
	memset(&_state, 0, sizeof(_state));
	
	if (x >= THRESHOLD) _state.right = true;
	if (x <= -THRESHOLD) _state.left = true;
	if (y >= THRESHOLD) _state.down = true;;
	if (y <= -THRESHOLD) _state.up = true;
	_state.fire = _joy.getButton(_fire);
}

