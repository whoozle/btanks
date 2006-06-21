#ifndef __BT_PLAYER_H__
#define __BT_PLAYER_H__

#include <SDL/SDL.h>
#include <sigc++/sigc++.h>

class Player {
public:
	float vx, vy;
	sigc::signal1<void, const std::string &> action;

	virtual void processEvent(const SDL_Event &event) = 0;
	virtual ~Player() {}
};

#endif

