
/* Battle Tanks Game
 * Copyright (C) 2006-2007 Battle Tanks team
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

#include "sdlx/sdlx.h"
#include "joyplayer.h"
#include "player_state.h"
#include "mrt/logger.h"

JoyPlayer::JoyPlayer(const int idx, const int fire, const int alt_fire, const int leave, const int hint_control)
: _joy(idx), _fire(fire), _alt_fire(alt_fire), leave(leave), _hint_control(hint_control) {
}

#define THRESHOLD 16384

void JoyPlayer::updateState(PlayerState &_state) {
	SDL_JoystickUpdate();
	Sint16 x = _joy.getAxis(0);
	Sint16 y = _joy.getAxis(1);
	
	_state.clear();	
	
	if (x >= THRESHOLD) _state.right = true;
	if (x <= -THRESHOLD) _state.left = true;
	if (y >= THRESHOLD) _state.down = true;
	if (y <= -THRESHOLD) _state.up = true;
	
	_state.fire = _joy.getButton(_fire);
	_state.alt_fire = _joy.getButton(_alt_fire);
	_state.leave = _joy.getButton(leave);
	_state.hint_control = _joy.getButton(_hint_control);
}

