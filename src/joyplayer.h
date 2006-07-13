#ifndef __BT_JOYPLAYER_H__
#define __BT_JOYPLAYER_H__

#include "object.h"
#include "sdlx/joystick.h"

class JoyPlayer :public Object {
public:
	JoyPlayer(const int idx);
	virtual void tick(const float dt);
	
	virtual ~JoyPlayer();
private:
	sdlx::Joystick _joy;
};

#endif

