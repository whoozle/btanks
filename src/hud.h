#ifndef __BTANKS_HUD_H__
#define __BTANKS_HUD_H__

#include "sdlx/surface.h"
#include "sdlx/font.h"

class Font;

class Hud {
public: 
	Hud();
	void render(sdlx::Surface &window);
	~Hud();

private: 
	sdlx::Surface _background;
	sdlx::Font _font;
};


#endif

