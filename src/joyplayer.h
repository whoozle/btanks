#ifndef __BT_JOYPLAYER_H__
#define __BT_JOYPLAYER_H__

#include "player.h"
#include "sdlx/joystick.h"

class JoyPlayer : public Player {
public:
	JoyPlayer(const int idx);
	virtual void processEvent(const SDL_Event &event) ;
	virtual ~JoyPlayer();
private:
	sdlx::Joystick _joy;
};

#endif

