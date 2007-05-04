#ifndef __BTANKS_CHEATER_H__
#define __BTANKS_CHEATER_H__

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

#include <string>
#include <vector>
#include <deque>

#include "sdlx/sdlx.h"
#include <sigc++/sigc++.h>

class Cheater : public sigc::trackable {
public: 
	Cheater();
private: 
	bool onKey(const SDL_keysym sym, const bool pressed);
	std::vector<std::string> _cheats;
	size_t _buf_size;
	char _buf[16];
};

#endif

