#ifndef __BTANKS_HUD_H__
#define __BTANKS_HUD_H__

#include "sdlx/surface.h"
#include "sdlx/font.h"
#include "alarm.h"

class Font;

class Hud {
public: 
	Hud(const int w, const int h);
	void render(sdlx::Surface &window) const;

	void renderSplash(sdlx::Surface &window) const;
	void renderLoadingBar(sdlx::Surface &window, const float progress) const;
	void renderRadar(const float dt, sdlx::Surface &window);

	~Hud();

private: 
	sdlx::Surface _background, _loading_border, _loading_item, _splash, _radar;
	sdlx::Font _font;
	Alarm _update_radar;
};


#endif

