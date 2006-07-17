#ifndef __BT_PLAYER_H__
#define __BT_PLAYER_H__
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

#include <SDL/SDL.h>
#include <sigc++/sigc++.h>

class Player {
public:
	float vx, vy;
	sigc::signal1<void, const std::string &> action;

	virtual void processEvent(const SDL_Event &event) = 0;
	virtual ~Player() {}
};

#endif

