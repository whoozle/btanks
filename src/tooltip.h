#ifndef BTANKS_TOOLTIP_H_
#define BTANKS_TOOLTIP_H_

#include "sdlx/surface.h"
#include <vector>

class Tooltip {
public: 
	Tooltip(const std::string &text);
	void render(sdlx::Surface &surface, const int x, const int y);
	void getSize(int &w, int &h);

private: 
	sdlx::Surface _surface;
	std::vector<int> _lines;
};

#endif

