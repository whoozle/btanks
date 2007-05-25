#ifndef __SDL_CXX_LAYER_RECT_H__
#define __SDL_CXX_LAYER_RECT_H__

/* sdlx - c++ wrapper for libSDL
 * Copyright (C) 2005-2007 Vladimir Menshakov
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

#include "sdlx.h"

namespace sdlx {
    class SDLXAPI Rect : public SDL_Rect {
    public:
	inline void reset() {
	    x = y = w = h = 0;
	}
	
	inline Rect() { x = y = w = h = 0; }
	
	inline Rect(int _x, int _y, int _w, int _h) {
		x = _x; y = _y; w = _w; h = _h;
	}
	
	inline const bool in(const int _x, const int _y) const {
		return (_x>=x && _y>=y && _x < x+w && _y < y+h);
	}
	
	inline const bool intersects(const Rect & other) const {
		return !( x >= (other.x + other.w) || (x + w) <= other.x ||
				y >= (other.y + other.h) || (y + h) <= other.y );
	}
	
	inline const bool inside(const Rect &other) const {
		return x >= other.x && x + w <= other.x + other.w &&
			y >= other.y && y + h <= other.y + other.h;
	}
};
}

#endif
