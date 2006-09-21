#include "sdlx/sdlx.h"
#include "joyplayer.h"
#include "player_state.h"
#include "mrt/logger.h"

JoyPlayer::JoyPlayer(const int idx, const int fire, const int alt_fire)
: _fire(fire), _alt_fire(alt_fire) {
	_joy.open(idx);
}

#define THRESHOLD 16384

void JoyPlayer::updateState(PlayerState &_state) {
	SDL_JoystickUpdate();
	Sint16 x = _joy.getAxis(0);
	Sint16 y = _joy.getAxis(1);
	
	_state.clear();	
	
	if (x >= THRESHOLD) _state.right = true;
	if (x <= -THRESHOLD) _state.left = true;
	if (y >= THRESHOLD) _state.down = true;;
	if (y <= -THRESHOLD) _state.up = true;
	_state.fire = _joy.getButton(_fire);
	_state.alt_fire = _joy.getButton(_alt_fire);
}

