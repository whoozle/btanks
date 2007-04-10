#ifndef BTANKS_JOYSTICK_SETUP_H__
#define BTANKS_JOYSTICK_SETUP_H__

#include "container.h"
#include "box.h"
#include "math/v2.h"

namespace sdlx {
	class Surface;
}
class Chooser;
class GamepadSetup : public Container {
public: 
	GamepadSetup(const int w, const int h);
	
	void load(const std::string &profile);
	void save();
	void tick(const float dt);

	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual void getSize(int &w, int &h) const;
	virtual bool onKey(const SDL_keysym sym);
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);

private: 
	Box _background; 
	Chooser *_current_pad;
	const sdlx::Surface *_gamepad_bg;
	v2<int> _gamepad_bg_pos;
	std::string _profile;
};

#endif

