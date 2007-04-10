#ifndef BTANKS_JOYSTICK_SETUP_H__
#define BTANKS_JOYSTICK_SETUP_H__

#include "container.h"
#include "box.h"
namespace sdlx {
	class Surface;
}
class Chooser;
class GamepadSetup : public Container {
public: 
	GamepadSetup(const int w, const int h);
	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual void getSize(int &w, int &h) const;
	virtual bool onKey(const SDL_keysym sym);

private: 
	Box _background; 
	Chooser *_current_pad;
	const sdlx::Surface *_gamepad_bg;
	int _gamepad_bg_y;
};

#endif

