#ifndef __BTANKS_WINDOW_H__
#define __BTANKS_WINDOW_H__

#include "sdlx/surface.h"

class Window {
public: 
	void init(const int argc, char *argv[]);
	void flip();
	void deinit();
protected:
	sdlx::Surface _window;
	bool _opengl;
};

#endif
