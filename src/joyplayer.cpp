#include "joyplayer.h"

JoyPlayer::JoyPlayer(const int idx) {
	_joy.open(idx);
}

void JoyPlayer::processEvent(const SDL_Event &event) {
	if (event.type != SDL_JOYAXISMOTION && event.type != SDL_JOYBUTTONDOWN && event.type != SDL_JOYBUTTONUP)
		return;
}

JoyPlayer::~JoyPlayer() {
	_joy.close();
}
