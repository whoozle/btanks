#include <SDL/SDL.h>
#include "joyplayer.h"
#include "resource_manager.h"
#include "mrt/logger.h"

REGISTER_OBJECT("joy-player", JoyPlayer, ());

JoyPlayer::JoyPlayer() : Player(false), _fire(0) {
	//LOG_WARN(("using button 0 [fixme]"));
}

JoyPlayer::JoyPlayer(const std::string &animation, const int idx, const int fire)
: Player(animation, false), _fire(fire) {
	_joy.open(idx);
}

Object * JoyPlayer::clone(const std::string &opt) const {
	JoyPlayer *p = NULL;
	TRY {
		p = new JoyPlayer(*this);
		LOG_WARN(("used 0 as joystick number and button index. [fixme]"));
		p->_joy.open(0);
	} CATCH("clone", { delete p; throw; })
	return p;
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

