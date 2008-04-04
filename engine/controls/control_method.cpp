#include "control_method.h"

ControlMethod::ControlMethod() : _release_set(false), _release_timer(false) {
	_release_timer.set(0.01f);
}

void ControlMethod::updateState(PlayerSlot &slot, PlayerState &state, const float dt) {
	_updateState(slot, state);
	return;
	
	if (state.compare_directions(_old_state)) {
		_old_state = state;
		return;
	}
	
	if (!_release_set) {
		LOG_DEBUG(("setting release timer"));
		_release_timer.reset();
		_release_set = true;
		return;
	} else if (_release_timer.tick(dt)) {
		LOG_DEBUG(("release timer stopped"));
		_old_state = state;
		_release_set = false;	
	} else {
		state = _old_state;
		return;
	}
}
