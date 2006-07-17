#ifndef __SDL_CXX_LAYER_RECT_H__
#define __SDL_CXX_LAYER_RECT_H__
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

namespace sdlx {
    class Rect : public SDL_Rect {
/*        typedef SDL_Rect* pSDL_Rect;
        typedef const SDL_Rect* pcSDL_Rect;
*/
    public:
        Rect() {
            x = y = w = h = 0;
        }
        Rect(int _x, int _y, int _w, int _h) {
            x = _x;
            y = _y;
            w = _w;
            h = _h;
        }
	
	void reset() {
	    x = y = w = h = 0;
	}
	inline const bool in(const int _x, const int _y) const {
	    return (_x>=x && _y>=y && 
		    _x < x+w && _y < y+h);
	}
	
	const bool intersected(const Rect & other) {
		int rx = x+w-1;
		int dy = y+h-1;
		
		return (
			other.in(x, y) ||
			other.in(rx, y) || 
			other.in(x, dy) ||
			other.in(rx, dy)
		);
	}
/*
        operator pSDL_Rect() {
            return &rect;
        }

        operator pcSDL_Rect() const {
            return &rect;
        }
*/
    };
}

#endif
