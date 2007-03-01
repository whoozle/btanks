#ifndef BTANKS_MENU_CONTROL_H__
#define BTANKS_MENU_CONTROL_H__

#include "sdlx/sdlx.h"

namespace sdlx {
	class Surface;
}

class Control {
public: 
	Control();
	virtual void tick(const float dt);
	virtual void render(sdlx::Surface &surface, const int x, const int y) = 0;
	virtual bool onKey(const SDL_keysym sym);
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);
	virtual ~Control() {}
	
	const bool changed() const { return _changed; } 
	void reset() { _changed = false; }
protected: 
	bool _changed;
};

#endif

