#include "mouse_control.h"
#include "mrt/logger.h"
#include "game.h"
#include "object.h"
#include "math/abs.h"

MouseControl::MouseControl(): _shoot(false) {
	Game->mouse_signal.connect(sigc::mem_fun(this, &MouseControl::onMouse));
} 

void MouseControl::onMouse(const int button, const bool pressed, const int x, const int y) {
	//LOG_DEBUG(("button %d,%d %d: %c", x, y, button, pressed?'+':'-'));
	if (button == 1) { //fixme: hardcoded
		_shoot = pressed;
	}
	if (!pressed)
		return;
	
	//LOG_DEBUG(("shoot: %c, move: %c", _shoot?'+':'-', _move?'+':'-'));
	Game->screen2world(_target, x, y);
	
	v3<float> pos;
	getPosition(pos);
	_velocity = _target - pos;
}

void MouseControl::updateState(PlayerState &state) {
	v3<float> pos;
	getPosition(pos);
	
	bool at_position = false;
	
	{
		v3<float> velocity = _target - pos;

/*		if (_shoot) {
			//prevent to moving for diagonal, find better position
			if (_velocity.x != 0 && _velocity.y != 0) {
				if (math::abs(_velocity.x) < math::abs(_velocity.y)) {
					_velocity.y = 0;
				} else if (math::abs(_velocity.y) < math::abs(_velocity.x)) {
					_velocity.x = 0;
				}
			}
		}
*/
		if (velocity.x * _velocity.x <= 0) {
			_velocity.x = 0;
			if (_shoot) 
				at_position = true;
		}
		if (velocity.y * _velocity.y <= 0) {
			_velocity.y = 0;
			if (_shoot) 
				at_position = true;
		}
	}
	state.fire = _shoot && at_position;
	if (at_position) {
		int dir = _velocity.getDirection8();
		if (dir)
			Game->getPlayerSlot(Game->getMyPlayerIndex()).obj->setDirection(dir - 1); //BIG FIXME!!
		_velocity.clear();
	}
	
	if (_velocity.x == 0) {
		state.left = state.right = false;
	} else {
		state.left = _velocity.x < 0;
		state.right = !state.left;
	}


	if (_velocity.y == 0) {
		state.up = state.down = false;
	} else {
		state.up = _velocity.y < 0;
		state.down = !state.up;
	}
}

void MouseControl::getPosition(v3<float>&pos) const {
	IGame::PlayerSlot &slot = Game->getPlayerSlot(Game->getMyPlayerIndex());
	slot.obj->getPosition(pos);
	pos += slot.obj->size / 2;
}

