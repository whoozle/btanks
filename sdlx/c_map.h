#ifndef __BTANK_COLLISION_MAP_H__
#define __BTANK_COLLISION_MAP_H__

#include "mrt/chunk.h"

namespace sdlx {
class Surface;
class Rect;

class CollisionMap {
public:
	CollisionMap();
	void init(const Surface * surface);
	const bool collides(const sdlx::Rect &src,  const CollisionMap *other, const sdlx::Rect &other_src, const int x, const int y) const;
	void save(const std::string &fname) const;
private: 
	unsigned int _w, _h;
	mrt::Chunk _data;
};
}

#endif

