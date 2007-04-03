#ifndef BTANKS_TOOLTIP_H_
#define BTANKS_TOOLTIP_H_

#include "sdlx/surface.h"
#include <vector>
#include "menu/box.h"

class Tooltip {
public: 
	Tooltip(const std::string &text, const bool use_background, const int w = 0);
	void render(sdlx::Surface &surface, const int x, const int y);
	void getSize(int &w, int &h);
	const float getReadingTime() const { return _time; }

private: 
	bool _use_background;
	Box _background;
	sdlx::Surface _surface;
	std::vector<int> _lines;
	float _time;
};

#endif

