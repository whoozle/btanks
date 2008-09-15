#ifndef BTANKS_SIMPLE_GAMEPAD_SETUP_H__
#define BTANKS_SIMPLE_GAMEPAD_SETUP_H__

#include "container.h"
#include "export_btanks.h"
#include "math/v2.h"

class Box;

class BTANKSAPI SimpleGamepadSetup : public Container {
public:
	SimpleGamepadSetup();
	virtual void render(sdlx::Surface &surface, const int x, const int y) const;
	void reload() {}
	void save() {}

	virtual bool onKey(const SDL_keysym sym);
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);
	virtual bool onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel);

private: 
	Box * bg;
	const sdlx::Surface *bg_table; 
	mutable const sdlx::Surface *selection;
	v2<int> bg_table_pos;
	int active_row;
};


#endif

