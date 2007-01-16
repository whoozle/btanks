
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
#include <queue>
#include <set>

#include "world.h"
#include "map.h"
#include "object.h"
#include "resource_manager.h"
#include "animation_model.h"
#include "mrt/random.h"

void ChainedDestructableLayer::onDeath(const int idx) {
	DestructableLayer::onDeath(idx);
	_slave->clear(idx);
}

void DestructableLayer::onDeath(const int idx) {
	_hp_data[idx] = -1;
	const int x = idx % _w, y = idx / _w;
	Map->invalidateTile(x, y);
	
	const sdlx::Surface *s = NULL;
	const sdlx::CollisionMap *cm = NULL;
	ResourceManager->checkSurface(ResourceManager.get_const()->getAnimation("building-explosion")->surface, s, cm);
	assert(s != NULL);
	
	Object * o = ResourceManager->createObject("explosion", "building-explosion");
	v3<int> tsize = Map->getTileSize();
	v3<float> pos(x * tsize.x + tsize.x/2, y * tsize.y + tsize.y/2, 0); //big fixme.
	pos -= o->size / 2;
	
	int dirs = (s->getWidth() - 1) / (int)o->size.x + 1;
	int dir = mrt::random(dirs);
	//LOG_DEBUG(("set dir %d (%d)", dir, dirs));
	o->setDirectionsNumber(dirs);
	o->setDirection(dir);
	
	World->addObject(o, pos);
}

DestructableLayer::DestructableLayer(const bool visible) : _hp_data(NULL), _visible(visible) {}

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
	const bool visible = _visible ? (_hp_data[i] == -1) : (_hp_data[i] > 0);
	if (i < 0 || i >= _w * _h || !visible)
		return 0;
	return *((Uint32 *) _data.getPtr() + i);
}

const sdlx::Surface* DestructableLayer::getSurface(const int x, const int y) const {
	int i = _w * y + x;
	const bool visible = _visible ? (_hp_data[i] == -1) : (_hp_data[i] > 0);
	if (i < 0 || i >= _w * _h || !visible)
		return NULL;
	return _tiles[i].surface;
}

const sdlx::CollisionMap* DestructableLayer::getCollisionMap(const int x, const int y) const {
	int i = _w * y + x;
	const bool visible = _visible ? (_hp_data[i] == -1) : (_hp_data[i] > 0);
	if (i < 0 || i >= _w * _h || !visible)
		return NULL;
	return _tiles[i].cmap;
}

const sdlx::CollisionMap* DestructableLayer::getVisibilityMap(const int x, const int y) const {
	int i = _w * y + x;
	const bool visible = _visible ? (_hp_data[i] == -1) : (_hp_data[i] > 0);
	if (i < 0 || i >= _w * _h || !visible)
		return NULL;
	return _tiles[i].vmap;
}

void DestructableLayer::damage(const int x, const int y, const int hp) {
	const int i = _w * y + x;
	if (i < 0 || i >= _w * _h)
		return;
	//LOG_DEBUG(("damage %d to cell %d", hp, i));
	if (_hp_data[i] <= 0) 
		return;
	
	_hp_data[i] -= hp;
	if (_hp_data[i] > 0)
		return;
		
	//_hp_data[i] = -1; //destructed cell
	const int size = _w * _h;
	
	std::queue<int> queue;
	std::set<int> visited;
	queue.push(i);
	while(!queue.empty()) {
		int v = queue.front();
		queue.pop();
		
		assert( v >= 0 && v < size );
		if (visited.find(v) != visited.end())
			continue;
		
		visited.insert(v);
		
		int x = v % _w, y = v / _w;
		//LOG_DEBUG(("checking %d %d -> %d", x, y, get(x, y)));
		if (Layer::get(x, y) == 0)
			continue;
		
		onDeath(v);
		
		if (x > 0)
			queue.push(v - 1);
		if (x < _w - 1)
			queue.push(v + 1);
		if (y > 0)
			queue.push(v - _w);
		if (y < _h - 1)
			queue.push(v + _w);
	}
	//LOG_DEBUG(("cleanup done"));
}

DestructableLayer::~DestructableLayer() {
	delete[] _hp_data;
}

Layer::Layer() : impassability(0), hp(0), pierceable(false), _tiles(NULL), _w(0), _h(0) {}

void Layer::init(const int w, const int h, const mrt::Chunk & data) {
	delete[] _tiles;
	_tiles = NULL;
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

void Layer::clear(const int i) {
	if (i < 0 || i >= _w * _h)
		return;
	_tiles[i].surface = NULL;
	_tiles[i].cmap = _tiles[i].vmap = NULL;
	*((Uint32 *) _data.getPtr() + i) = 0;
}


const sdlx::Surface* Layer::getSurface(const int x, const int y) const {
	if (x < 0 || x >= _w || y < 0 || y >= _h) 
		return NULL;
	return _tiles[_w * y + x].surface;
}

const sdlx::CollisionMap* Layer::getCollisionMap(const int x, const int y) const {
	if (x < 0 || x >= _w || y < 0 || y >= _h) 
		return NULL;
	return _tiles[_w * y + x].cmap;
}

const sdlx::CollisionMap* Layer::getVisibilityMap(const int x, const int y) const {
	if (x < 0 || x >= _w || y < 0 || y >= _h) 
		return NULL;
	return _tiles[_w * y + x].vmap;
}


void Layer::optimize(const IMap::TileMap & tilemap) {
	unsigned size = _w * _h;
	
	Uint32 *ptr = (Uint32 *)_data.getPtr();

	delete[] _tiles;
	_tiles = new IMap::TileDescriptor[size];

	for(unsigned int i = 0; i < size; ++i) {
		Uint32 tid = *ptr++;

		if (tid != 0) { 
			if ((unsigned)tid >= tilemap.size())
				throw_ex(("got invalid tile id %d", tid));
			_tiles[i] = tilemap[tid];
		}
	}
}

void Layer::damage(const int x, const int y, const int hp) {}


Layer::~Layer() { delete[] _tiles; }
