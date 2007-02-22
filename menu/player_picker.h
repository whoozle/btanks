#ifndef BTANKS_PLAYER_PICKER_H__
#define BTANKS_PLAYER_PICKER_H__

#include "container.h"
#include "box.h"

class PlayerPicker : public Container {
public: 
	PlayerPicker(const int w, const int h, const int slots);
	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual bool onKey(const SDL_keysym sym) ;
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);
private: 
	Box _background;
	int _slots;
};

#endif

