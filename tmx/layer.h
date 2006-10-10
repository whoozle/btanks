#ifndef __BT_LAYER_H__
#define __BT_LAYER_H__

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


#include "mrt/chunk.h"
#include "sdlx/surface.h"
#include "sdlx/c_map.h"
#include <vector>

#define PRERENDER_LAYERS
#undef PRERENDER_LAYERS


class Layer {
public:
#ifdef PRERENDER_LAYERS
	sdlx::Surface surface;
#endif
	const int impassability;
	const bool pierceable;

	Layer(const int w, const int h, const mrt::Chunk & data, const int impassability, const bool pierceable);

	inline const Uint32 get(const int x, const int y) const {
		if (x < 0 || x >= _w || y < 0 || y >= _h) 
			return 0;	
		return *((Uint32 *) _data.getPtr() + _w * y + x);
	}

	inline const sdlx::Surface* getSurface(const int x, const int y) const {
		if (x < 0 || x >= _w || y < 0 || y >= _h) 
			return NULL;
		return _s_data[_w * y + x];
	}

	inline const sdlx::CollisionMap* getCollisionMap(const int x, const int y) const {
		if (x < 0 || x >= _w || y < 0 || y >= _h) 
			return NULL;
		return _c_data[_w * y + x];
	}

	void optimize(std::vector< std::pair< sdlx::Surface *, sdlx::CollisionMap *> > & tilemap);
	~Layer();

private: 
	mrt::Chunk _data;
	sdlx::Surface **_s_data;
	sdlx::CollisionMap **_c_data;
	int _w, _h;
};

#endif

