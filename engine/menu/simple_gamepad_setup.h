#ifndef BTANKS_SIMPLE_GAMEPAD_SETUP_H__
#define BTANKS_SIMPLE_GAMEPAD_SETUP_H__

#include "container.h"
#include "math/v2.h"
#include "sl08/sl08.h"
#include "sdlx/joystick.h"
#include "controls/simple_joy_bindings.h"

class Box;
class Chooser;
class Slider;
class Label;

class SimpleGamepadSetup : public Container {
public:
	SimpleGamepadSetup();
	virtual void render(sdlx::Surface &surface, const int x, const int y) const;
	void reload() {}
	void save() {}
	void hide(const bool hide = true);
	
	void init(const int idx);

private: 
	virtual void tick(const float dt);
	virtual bool onKey(const SDL_keysym sym);
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);
	virtual bool onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel);
	void refresh();

	sl08::slot1<void, const SDL_Event &, SimpleGamepadSetup> on_event_slot;
	virtual void on_event(const SDL_Event &event);

	Box * bg;
	const sdlx::Surface *bg_table; 
	mutable const sdlx::Surface *selection;
	Chooser *joy_list;
	Slider * dead_zone;
	Label *controls[8];
	
	v2<int> bg_table_pos;
	int active_row;

	sdlx::Joystick joy;
	std::string profile;
	SimpleJoyBindings bindings;
};


#endif

