#ifndef BTANKS_BOX_H__
#define BTANKS_BOX_H__

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
#include "export_btanks.h"
#include "control.h"

namespace sdlx {
class Surface;
}

class BTANKSAPI Box : public Control{
public: 
	Box() : _surface(0) {}
	Box(const std::string &tile, int w, int h);
	Box(const std::string &tile, const std::string &highlight, int w, int h);
	virtual void getSize(int &w, int &h) const;
	int w, h;

	const bool inited() const { return _surface != 0; }
	void init(const std::string &tile, int w, int h);
	void init(const std::string &tile, const std::string &highlight_tile, int w, int h);
	
	virtual void render(sdlx::Surface &surface, const int x, const int y);
	void copyTo(sdlx::Surface &surface, const int x, const int y);
	virtual void renderHL(sdlx::Surface &surface, const int x, const int y);
	virtual ~Box() {}
	
	void getMargins(int &v, int &h) const;
private: 
	int x1, x2, y1, y2, xn, yn;
	
	const sdlx::Surface *_surface, *_highlight;
};

#endif

