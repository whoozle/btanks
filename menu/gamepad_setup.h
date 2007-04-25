#ifndef BTANKS_JOYSTICK_SETUP_H__
#define BTANKS_JOYSTICK_SETUP_H__

#include "container.h"
#include "box.h"
#include "alarm.h"
#include "math/v2.h"
#include "sdlx/joystick.h"

#include <map>

namespace sdlx {
	class Surface;
}
class Chooser;
class Button;

class GamepadSetup : public Container {
public: 
	GamepadSetup(const int w, const int h);
	
	void load(const std::string &profile);
	void reload();
	void save();
	void tick(const float dt);

	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual void getSize(int &w, int &h) const;

private: 
	virtual void renderSetup(sdlx::Surface &surface, const int x, const int y);
	virtual bool onKey(const SDL_keysym sym);
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);
	virtual void onEvent(const SDL_Event &event);

	void renderIcon(sdlx::Surface &surface, const int idx, const int x, const int y);
	void renderDPad(sdlx::Surface &surface, const bool left, const bool right, const bool up, const bool down, const int x, const int y);
	void renderButton(sdlx::Surface &surface, const int b, const int x, const int y);
	void renderMinistick(sdlx::Surface &surface, const int ai, const int x, const int y);

	enum ControlType {
		tButton = 1, tAxis = 2, tHat = 3
	};
	
	Box _background; 
	Chooser *_current_pad;
	Button *_setup, *_back;
	const sdlx::Surface *_gamepad_bg, *_gamepad_buttons, *_gamepad_ministick;
	v2<int> _gamepad_bg_pos;
	std::string _profile;
	
	sdlx::Joystick joy;

//interactive setup part
	void setup();
	void setupNextControl();

	bool _wait;
	Alarm _blink;
	ControlType _wait_control, _got_control;
	int _control_id;
	typedef std::map<const std::pair<ControlType, int> , int> Bindings;
	Bindings _bindings;
};

#endif

