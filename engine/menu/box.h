#ifndef BTANKS_BOX_H__
#define BTANKS_BOX_H__

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

#include <string>
#include "export_btanks.h"
#include "control.h"
#include "sdlx/surface.h"

namespace sdlx {
class Surface;
}

class BTANKSAPI Box : public Control{
public: 
	Box() : w(0), h(0), _surface(0) {}
	Box(const std::string &tile, int w, int h);
	Box(const std::string &tile, int w, int h, int hl_h);
	virtual void get_size(int &rw, int &rh) const;
	int w, h;

	const bool inited() const { return _surface != 0; }
	void init(const std::string &tile, int w, int h, int hl_h = 0);
	void set_background(const std::string &tile);
	inline const std::string get_background() const { return bg_tile; }
	
	virtual void render(sdlx::Surface &surface, const int x, const int y) const;
	void copyTo(sdlx::Surface &surface, const int x, const int y);
	virtual void renderHL(sdlx::Surface &surface, const int x, const int y) const;
	virtual ~Box() {}
	
	void getMargins(int &v, int &h) const;
	void setHLColor(int r, int g, int b, int a);
private: 
	int x1, x2, y1, y2, xn, yn;
	std::string bg_tile;

	const sdlx::Surface *_surface;
	sdlx::Surface _filler, _filler_u, _filler_d, _filler_l, _filler_r;
	sdlx::Surface _highlight;
};

#endif

