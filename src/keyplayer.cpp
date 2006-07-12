#include "keyplayer.h"
#include "mrt/logger.h"

void KeyPlayer::processEvent(const SDL_Event &event) {
	switch(event.type) {
		case SDL_JOYAXISMOTION: 
		break;
	}
}

KeyPlayer::~KeyPlayer() {}
