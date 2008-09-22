#ifndef BTANKS_MENU_MEDALS_H__
#define BTANKS_MENU_MEDALS_H__

#include "container.h"

namespace sdlx {
	class Surface;
}

class Campaign;
class Medals : public Container {
public: 
	Medals(int w, int h); 
	bool onKey(const SDL_keysym sym);
	virtual void hide(const bool hide = true);
	void set(const Campaign * c) { campaign = c; }
private: 
	const Campaign * campaign;
	std::vector<const sdlx::Surface *> tiles;
};

#endif

