
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

#include <string.h>
#include <assert.h>

#include "mrt/logger.h"

#include "keyplayer.h"
#include "object.h"
#include "world.h"
#include "config.h"

bool KeyPlayer::_disabled;

KeyPlayer::KeyPlayer(const std::string &variant) {
	int up, down, left, right, fire, alt_fire, leave, hint_control;

#include "controls/default_keys.cpp"
	int i = 0;
	if (variant == "keys") 
		i = 0;
	else if (variant == "keys-1")
		i = 1;
	else if (variant == "keys-2") 
		i = 2;
	else throw_ex(("unknown keyboard variant used (%s)", variant.c_str()));

	Config->get("player.controls." + variant + ".up", up, keys[i][0]);
	Config->get("player.controls." + variant + ".down", down, keys[i][1]);
	Config->get("player.controls." + variant + ".left", left, keys[i][2]);
	Config->get("player.controls." + variant + ".right", right, keys[i][3]);
	Config->get("player.controls." + variant + ".fire", fire, keys[i][4]);
	Config->get("player.controls." + variant + ".alt-fire", alt_fire, keys[i][5]);
	Config->get("player.controls." + variant + ".disembark", leave, keys[i][6]);
	Config->get("player.controls." + variant + ".hint-control", hint_control, keys[i][7]);

	_up = (SDLKey)up;
	_down = (SDLKey)down;
	_left = (SDLKey)left;
	_right = (SDLKey)right;
	_fire = (SDLKey)fire;
	_alt_fire = (SDLKey)alt_fire;
	this->leave = (SDLKey)leave;
	_hint_control = (SDLKey)hint_control;
}

void KeyPlayer::_updateState(PlayerSlot &slot, PlayerState &state, const float dt) {
	if (_disabled)
		return;
	
	static const Uint8 *keys = SDL_GetKeyState(0);
	state.left = keys[_left] != 0;
	state.right = keys[_right] != 0;
	state.up = keys[_up] != 0;
	state.down = keys[_down] != 0;
	state.fire = keys[_fire] != 0;
	state.alt_fire = keys[_alt_fire] != 0;
	state.leave = keys[leave] != 0;
	state.hint_control = keys[_hint_control] != 0;
}

#define CHECKSTATE(name, member) if (state.name) {\
	const char *name = SDL_GetKeyName(this->member); \
	controls.push_back(mrt::format_string("[%s]", name?name: "unknown")); \
}

void KeyPlayer::get_name(std::vector<std::string> &controls, const PlayerState &state) const {
	CHECKSTATE(left, _left);
	CHECKSTATE(right, _right);
	CHECKSTATE(up, _up);
	CHECKSTATE(down, _down);
	CHECKSTATE(fire, _fire);
	CHECKSTATE(alt_fire, _alt_fire);
	CHECKSTATE(leave, leave);
	CHECKSTATE(hint_control, _hint_control);
}
