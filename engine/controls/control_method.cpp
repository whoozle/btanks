#include "control_method.h"

ControlMethod::ControlMethod() : _release_set(false), _release_timer(false) {
	_release_timer.set(0.075f);
}

void ControlMethod::updateState(PlayerSlot &slot, PlayerState &state, const float dt) {
	_updateState(slot, state, dt);
	
	int dirs = 0;
	dirs += state.left;
	dirs += state.right;
	dirs += state.up;
	dirs += state.down;

	int old_dirs = 0;
	old_dirs += _old_state.left;
	old_dirs += _old_state.right;
	old_dirs += _old_state.up;
	old_dirs += _old_state.down;
	
	if (state.compare_directions(_old_state) || (old_dirs != 2 || dirs != 1)) {
		_old_state = state;
		return;
	}
	
	if (!_release_set) {
		//LOG_DEBUG(("setting release timer"));
		_release_timer.reset();
		_release_set = true;
		state = _old_state;
		return;
	} else if (_release_timer.tick(dt)) {
		//LOG_DEBUG(("release timer stopped"));
		_old_state = state;
		_release_set = false;	
	} else {
		if ((!state.left && _old_state.left) || (!state.right && _old_state.right)) {
			if ((!state.up && _old_state.up) || (!state.down && _old_state.down)) {
				LOG_DEBUG(("atomically update diagonal"));
				_old_state = state;
				_release_set = false;
				return;
			}
		}
		state = _old_state;
		return;
	}
}

const std::string ControlMethod::get_name(PlayerState& state) const {
	std::vector<std::string> c;
	get_name(c, state);
	std::string r;
	mrt::join(r, c, "+");
	mrt::replace(r, " ", "\\s");
	return r;
}
