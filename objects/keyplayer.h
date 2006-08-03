#ifndef __BT_KEYPLAYER_H__
#define __BT_KEYPLAYER_H__
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

#include "sdlx/joystick.h"
#include "player.h"

class AnimatedObject;

class KeyPlayer : public Player {
public:
	KeyPlayer(const std::string &animation, SDLKey up, SDLKey down, SDLKey left, SDLKey right, SDLKey fire);
	virtual ~KeyPlayer();

private:
	void onKey(const Uint8 type, const SDL_keysym sym);
	SDLKey _up, _down, _left, _right, _fire;
};

#endif

