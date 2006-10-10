
/* Battle Tanks Game
 * Copyright (C) 2006 Battle Tanks team
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

#include <string.h>

#include "mrt/logger.h"

#include "keyplayer.h"
#include "object.h"
#include "world.h"
#include <assert.h>

KeyPlayer::KeyPlayer(SDLKey up, SDLKey down, SDLKey left, SDLKey right, SDLKey fire, SDLKey alt_fire): 
	_up(up), _down(down), _left(left), _right(right), _fire(fire), _alt_fire(alt_fire) {
}

void KeyPlayer::updateState(PlayerState &state) {
	static const Uint8 *keys = SDL_GetKeyState(0);
	state.left = keys[_left] != 0;
	state.right = keys[_right] != 0;
	state.up = keys[_up] != 0;
	state.down = keys[_down] != 0;
	state.fire = keys[_fire] != 0;
	state.alt_fire = keys[_alt_fire] != 0;
}
