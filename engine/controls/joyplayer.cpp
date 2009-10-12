
/* Battle Tanks Game
 * Copyright (C) 2006-2009 Battle Tanks team
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
#include "window.h"

JoyPlayer::JoyPlayer(const int idx): _idx(idx), _joy(idx) {
	event_slot.assign(this, &JoyPlayer::on_event, Window->event_signal);
	_name = sdlx::Joystick::getName(idx);
	_bindings = SimpleJoyBindings(_name, _joy);
}

#define THRESHOLD 16384

void JoyPlayer::on_event(const SDL_Event &event) {
	if (
		((event.type == SDL_JOYBUTTONDOWN || event.type == SDL_JOYBUTTONUP) && event.jbutton.which == _idx) ||
		(event.type == SDL_JOYAXISMOTION && event.jaxis.which == _idx) ||
		(event.type == SDL_JOYHATMOTION && event.jhat.which == _idx)
	) {
		_bindings.update(_state, event);
	}
}

void JoyPlayer::_updateState(PlayerSlot &slot, PlayerState &state, const float dt) {
	state = _state;
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
