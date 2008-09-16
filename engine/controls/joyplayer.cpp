
/* Battle Tanks Game
 * Copyright (C) 2006-2008 Battle Tanks team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
*/

#include "sdlx/sdlx.h"
#include "joyplayer.h"
#include "player_state.h"
#include "player_slot.h"
#include "mrt/logger.h"
#include "config.h"

JoyPlayer::JoyPlayer(const int idx): _idx(idx), _joy(idx) {
	_name = sdlx::Joystick::getName(idx);
	_bindings = SimpleJoyBindings(_name, _joy);
}

#define THRESHOLD 16384

void JoyPlayer::_updateState(PlayerSlot &slot, PlayerState &_state, const float dt) {
	SDL_JoystickUpdate();
	
	_bindings.update(_state, _joy);
/*	
	if (x >= THRESHOLD) _state.right = true;
	if (x <= -THRESHOLD) _state.left = true;
	if (y >= THRESHOLD) _state.down = true;
	if (y <= -THRESHOLD) _state.up = true;
	
	_state.fire = _joy.get_button(_bindings.get(tButton, 0)) || _joy.get_button(_bindings.get(tButton, 5));
	_state.alt_fire = _joy.get_button(_bindings.get(tButton, 1))  || _joy.get_button(_bindings.get(tButton, 6));
	_state.leave = _joy.get_button(_bindings.get(tButton, 3));
	_state.hint_control = _joy.get_button(_bindings.get(tButton, 4));

	int r;
	Config->get("player.controls.maximum-camera-slide", r, 200);
	
	bool im2x;
	Config->get(mrt::format_string("player.controls.joystick.%s.ignore-more-than-two-axis", _name.c_str()), im2x, false);
	if (im2x)
		return;
	
	int n = _joy.get_axis_num();
	if (n >= 4) {
		int xa = _joy.get_axis(_bindings.get(tAxis, 2));
		int ya = _joy.get_axis(_bindings.get(tAxis, 3));

		slot.map_dpos.x = (xa * r) / 32767;
		slot.map_dpos.y = (ya * r) / 32767;
	}
*/
}

void JoyPlayer::get_name(std::vector<std::string> &controls, const PlayerState &state) const {
	if (state.fire) { 
		controls.push_back(_bindings.get_name(4));
	}
	if (state.alt_fire) {
		controls.push_back(_bindings.get_name(5));
	}
	if (state.leave) {
		controls.push_back(_bindings.get_name(6));
	}
	if (state.hint_control) {
		controls.push_back(_bindings.get_name(7));
	}
}
