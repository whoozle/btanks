#include "control_method.h"

void ControlMethod::updateState(PlayerSlot &slot, PlayerState &state, const float dt) {
	_updateState(slot, state);
	
	old_state = state;
}
