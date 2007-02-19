#ifndef BTANKS_BASE_MENU_H__
#define BTANKS_BASE_MENU_H__

#include "sdlx/sdlx.h"

namespace sdlx {
	class Surface;
}


class BaseMenu {
public:
	virtual void render(sdlx::Surface &dst) = 0;
	virtual bool onKey(const SDL_keysym sym) = 0;

	virtual ~BaseMenu() {}
};

#endif

