
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

#include "layer.h"
#include <assert.h>
#include "mrt/exception.h"
#include <assert.h>

Layer::Layer(const int w, const int h, const mrt::Chunk & data, const int impassability, const bool pierceable) : 
impassability(impassability), pierceable(pierceable), _s_data(NULL), _c_data(NULL), _w(w), _h(h) {
	_data = data;
	assert((int)_data.getSize() == (4 * _w * _h));
}

void Layer::optimize(std::vector< std::pair<sdlx::Surface *, sdlx::CollisionMap *> > & tilemap) {
	unsigned size = _w * _h;
	
	Uint32 *ptr = (Uint32 *)_data.getPtr();
	delete _s_data;
	_s_data = new sdlx::Surface*[size];
	delete _c_data;
	_c_data = new sdlx::CollisionMap*[size];

	for(unsigned int i = 0; i < size; ++i) {
		Uint32 tid = *ptr++;

		if (tid == 0) { 
			_s_data[i] = 0;
			_c_data[i] = 0;
		} else {
			if ((unsigned)tid >= tilemap.size())
				throw_ex(("got invalid tile id %d", tid));
			_s_data[i] = tilemap[tid].first;
			_c_data[i] = tilemap[tid].second;
		}
	}
}

Layer::~Layer() { delete[] _s_data; delete[] _c_data; }
