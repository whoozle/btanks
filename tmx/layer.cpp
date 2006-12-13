
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

void ChainedDestructableLayer::damage(const int x, const int y, const int hp) {
	int i = _w * y + x;
	if (i < 0 || i >= _w * _h)
		return;
	//LOG_DEBUG(("damage %d to cell %d", hp, i));
	if (_hp_data[i] > 0) {
		_hp_data[i] -= hp;
		if (_hp_data[i] <= 0) {
			_hp_data[i] = -1; //destructed cell
			//LOG_DEBUG(("clearing slave cell"));
			_slave->clear(x, y);
		}
	}
}


DestructableLayer::DestructableLayer() : _hp_data(NULL) {}

void DestructableLayer::init(const int w, const int h, const mrt::Chunk & data) {
	if (hp <= 0)
		throw_ex(("destructable layer cannot have hp %d (hp must be > 0)", hp));
	Layer::init(w, h, data);
	
	int size = _w * _h;
	delete[] _hp_data;
	_hp_data = new int[size];

	Uint32 *ptr = (Uint32 *)_data.getPtr();
	for(int i = 0; i < size; ++i) {
		_hp_data[i] = (ptr[i] != 0) ? hp : 0;
	}
}
const Uint32 DestructableLayer::get(const int x, const int y) const {
	int i = _w * y + x;
	if (i < 0 || i >= _w * _h || _hp_data[i] != -1)
		return 0;
	return *((Uint32 *) _data.getPtr() + i);
}

const sdlx::Surface* DestructableLayer::getSurface(const int x, const int y) const {
	int i = _w * y + x;
	if (i < 0 || i >= _w * _h || _hp_data[i] != -1)
		return NULL;
	return _s_data[i];
}

const sdlx::CollisionMap* DestructableLayer::getCollisionMap(const int x, const int y) const {
	int i = _w * y + x;
	if (i < 0 || i >= _w * _h || _hp_data[i] != -1)
		return NULL;
	return _c_data[i];
}

void DestructableLayer::damage(const int x, const int y, const int hp) {
	int i = _w * y + x;
	if (i < 0 || i >= _w * _h)
		return;
	//LOG_DEBUG(("damage %d to cell %d", hp, i));
	if (_hp_data[i] > 0) {
		_hp_data[i] -= hp;
		if (_hp_data[i] <= 0) 
			_hp_data[i] = -1; //destructed cell
	}
}

DestructableLayer::~DestructableLayer() {
	delete[] _hp_data;
}

Layer::Layer() : impassability(0), hp(0), pierceable(false), visible(false), _s_data(NULL), _c_data(NULL), _w(0), _h(0) {}

void Layer::init(const int w, const int h, const mrt::Chunk & data) {
	delete[] _s_data;
	_s_data = NULL;
	delete[] _c_data;
	_c_data = NULL;
	_w = w;
	_h = h;
	_data = data;
	assert((int)_data.getSize() == (4 * _w * _h));
}


const Uint32 Layer::get(const int x, const int y) const {
	if (x < 0 || x >= _w || y < 0 || y >= _h) 
		return 0;	
	return *((Uint32 *) _data.getPtr() + _w * y + x);
}

void Layer::clear(const int x, const int y) {
	int i = _w * y + x;
	if (i < 0 || i >= _w * _h)
		return;
	_s_data[i] = NULL;
	_c_data[i] = NULL;
	*((Uint32 *) _data.getPtr() + i) = 0;
}


const sdlx::Surface* Layer::getSurface(const int x, const int y) const {
	if (x < 0 || x >= _w || y < 0 || y >= _h) 
		return NULL;
	return _s_data[_w * y + x];
}

const sdlx::CollisionMap* Layer::getCollisionMap(const int x, const int y) const {
	if (x < 0 || x >= _w || y < 0 || y >= _h) 
		return NULL;
	return _c_data[_w * y + x];
}


void Layer::optimize(std::vector< std::pair<sdlx::Surface *, sdlx::CollisionMap *> > & tilemap) {
	unsigned size = _w * _h;
	
	Uint32 *ptr = (Uint32 *)_data.getPtr();
	delete[] _s_data;
	_s_data = new sdlx::Surface*[size];
	delete[] _c_data;
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

void Layer::damage(const int x, const int y, const int hp) {}


Layer::~Layer() { delete[] _s_data; delete[] _c_data; }
