#ifndef BTANKS_TOOLTIP_H_
#define BTANKS_TOOLTIP_H_

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

#include "sdlx/surface.h"
#include <vector>
#include "menu/box.h"

class Tooltip {
public: 
	Tooltip(const std::string &text, const bool use_background, const int w = 0);
	void render(sdlx::Surface &surface, const int x, const int y);
	void getSize(int &w, int &h);
	const float getReadingTime() const { return _time; }

private: 
	bool _use_background;
	Box _background;
	sdlx::Surface _surface;
	std::vector<int> _lines;
	float _time;
};

#endif

