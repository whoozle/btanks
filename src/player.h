#ifndef __BT_PLAYER_H__
#define __BT_PLAYER_H__

#include <SDL/SDL.h>

class Player {
public:
	virtual void processEvent(const SDL_Event &event) = 0;
	virtual ~Player() {}
};

#endif

