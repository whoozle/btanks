#ifndef __BTANKS_WORLDMAP_H__
#define __BTANKS_WORLDMAP_H__

#include "v3.h"

namespace sdlx {
class Surface;
}

class WorldMap {
public:
	virtual const int getImpassability(const sdlx::Surface &object_surf, const v3<int> &pos) const = 0;
	virtual ~WorldMap() {}
};

#endif

