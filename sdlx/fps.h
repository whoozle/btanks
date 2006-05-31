#ifndef __SDLX_FRAMERATE_H__
#define __SDLX_FRAMERATE_H__

#include "SDL/SDL_framerate.h"

namespace sdlx {
class FPSManager {
public:
	FPSManager();
	void set(const int rate);
	const int get();
	void delay();
private:
	FPSmanager _fps;
//	unsigned _last_tick, _rate;
//	unsigned _last_fps;
};
}

#endif

