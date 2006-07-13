#ifndef __BT_KEYPLAYER_H__
#define __BT_KEYPLAYER_H__

#include "object.h"
#include "sdlx/joystick.h"

class KeyPlayer : public Object {
public:
	KeyPlayer();
	virtual ~KeyPlayer();
	virtual void tick(const float dt);
	
protected: 
	struct state { float vx, vy; } state;
private:
	void onKey(const Uint8 type, const SDL_keysym sym);
};

#endif

