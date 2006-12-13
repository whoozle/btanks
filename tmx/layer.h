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
	typedef std::vector< std::pair< sdlx::Surface *, sdlx::CollisionMap *> > TileData;
#ifdef PRERENDER_LAYERS
	sdlx::Surface surface;
#endif
	int impassability, hp;
	bool pierceable, visible;

	Layer();
	virtual void init(const int w, const int h, const mrt::Chunk & data);

	void clear(const int idx);
	
	virtual const Uint32 get(const int x, const int y) const; 
	virtual const sdlx::Surface* getSurface(const int x, const int y) const;
	virtual const sdlx::CollisionMap* getCollisionMap(const int x, const int y) const;
	virtual void damage(const int x, const int y, const int hp);

	void optimize(TileData & tilemap);
	virtual ~Layer();

protected: 
	mrt::Chunk _data;
	sdlx::Surface **_s_data;
	sdlx::CollisionMap **_c_data;
	int _w, _h;
};

class DestructableLayer : public Layer {
public: 
	DestructableLayer();
	virtual void init(const int w, const int h, const mrt::Chunk & data);

	virtual const Uint32 get(const int x, const int y) const; 
	virtual const sdlx::Surface* getSurface(const int x, const int y) const;
	virtual const sdlx::CollisionMap* getCollisionMap(const int x, const int y) const;

	virtual void damage(const int x, const int y, const int hp);
	virtual void onDeath(const int idx);
	
	~DestructableLayer();
protected:
	int *_hp_data;
};

class ChainedDestructableLayer : public DestructableLayer {
public: 
	ChainedDestructableLayer() : _slave(NULL) {}
	void setSlave(Layer *layer) { _slave = layer; }

	virtual void onDeath(const int idx);
private: 
	Layer *_slave;
};

#endif

