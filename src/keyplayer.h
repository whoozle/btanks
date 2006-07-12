#ifndef __BT_KEYPLAYER_H__
#define __BT_KEYPLAYER_H__

#include "player.h"
#include "sdlx/joystick.h"

class KeyPlayer : public Player {
public:
	virtual void processEvent(const SDL_Event &event) ;
	virtual ~KeyPlayer();
};

#endif

