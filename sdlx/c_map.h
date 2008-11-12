#ifndef __BTANK_COLLISION_MAP_H__
#define __BTANK_COLLISION_MAP_H__

/* sdlx - c++ wrapper for libSDL
 * Copyright (C) 2005-2007 Vladimir Menshakov
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "mrt/chunk.h"
#include "export_sdlx.h"

template <class T> 
	class Matrix;

namespace sdlx {
class Surface;
class Rect;

class SDLXAPI CollisionMap {
public:
	enum Type {OnlyOpaque, AnyVisible};
	
	CollisionMap();
	void init(const Surface * surface, const Type type);
	void create(const unsigned int w, const unsigned int h, const bool bit);
	bool load(unsigned int w, unsigned int h, const mrt::Chunk &data);
	void project(Matrix<bool> &result, const unsigned w, const unsigned h) const;
	const bool collides(const sdlx::Rect &src,  const CollisionMap *other, const sdlx::Rect &other_src, const int x, const int y, const bool hidden_by_other) const;
	void save(const std::string &fname) const;
	
	inline const bool is_empty() const { return _empty; }
	inline const bool is_full() const { return _full; }
	inline const std::string dump() const { return _data.dump();}
private: 
	bool _empty, _full;
	unsigned int _w, _h;
	mrt::Chunk _data;
};
}

#endif

