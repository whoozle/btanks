#include <SDL/SDL.h>
#include "joyplayer.h"
#include "mrt/logger.h"

JoyPlayer::JoyPlayer(const std::string &animation, const int idx, const int fire)
: Player(animation, false), _fire(fire) {
	_joy.open(idx);
}

#define THRESHOLD 16384

void JoyPlayer::tick(const float dt) {
	SDL_JoystickUpdate();
	Sint16 x = _joy.getAxis(0);
	Sint16 y = _joy.getAxis(1);
	
	memset(&_state, 0, sizeof(_state));
	
	if (x >= THRESHOLD) _state.right = true;
	if (x <= -THRESHOLD) _state.left = true;
	if (y >= THRESHOLD) _state.down = true;;
	if (y <= -THRESHOLD) _state.up = true;
	_state.fire = _joy.getButton(_fire);

	Player::tick(dt);
}

/*
void JoyPlayer::processEvent(const SDL_Event &event) {
	switch(event.type) {
		case SDL_JOYAXISMOTION: 
			//LOG_DEBUG(("%d:%d", event.jaxis.axis, event.jaxis.value));
			if (event.jaxis.axis == 0) {
				vx = 0;
				if (event.jaxis.value >= THRESHOLD)  vx += 1;
				if (event.jaxis.value <= -THRESHOLD) vx -= 1;
			}

			if (event.jaxis.axis == 1) {
				vy = 0;
				if (event.jaxis.value >= THRESHOLD)  vy += 1;
				if (event.jaxis.value <= -THRESHOLD) vy -= 1;
			}
			//LOG_DEBUG(("(vx, vy) = (%f, %f)", vx, vy));
		break;
		case SDL_JOYBUTTONDOWN: 
			//LOG_DEBUG(("button: %d", event.jbutton.button));
			switch(event.jbutton.button) {
				case 0: action.emit("shoot");
			}
		break;
		case SDL_JOYBUTTONUP: 
		break;
	}
}
*/

JoyPlayer::~JoyPlayer() {
	_joy.close();
}
