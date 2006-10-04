#ifndef __BTANKS_MOUSE_CONTROL_H__
#define __BTANKS_MOUSE_CONTROL_H__

#include "sdlx/joystick.h"
#include "control_method.h"
#include "player_state.h"
#include "math/v3.h"

class MouseControl : public ControlMethod {
public:
	MouseControl(); 
	virtual void updateState(PlayerState &state);
private:
	void getPosition(v3<float>&pos) const;
	
	void onMouse(const int button, const bool pressed, const int x, const int y);
	v3<float> _velocity, _target;
	bool _shoot;
};

#endif
