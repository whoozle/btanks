#ifndef __BTANK_COLLISION_MAP_H__
#define __BTANK_COLLISION_MAP_H__

#include "mrt/chunk.h"

namespace sdlx {
class Surface;

class CollisionMap {
public:
	CollisionMap();
	void init(const Surface * surface);
	const bool collides(const CollisionMap *other, const int x, const int y) const;
private: 
	unsigned int _w, _h;
	mrt::Chunk _data;
};
}

#endif

