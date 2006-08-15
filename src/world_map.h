#ifndef __BTANKS_WORLDMAP_H__
#define __BTANKS_WORLDMAP_H__

#include "math/v3.h"
#include "math/graph.h"

namespace sdlx {
class Surface;
}

class WorldMap {
public:
	virtual const int getImpassability(const sdlx::Surface &object_surf, const v3<int> &pos) const = 0;
	virtual ~WorldMap() {}
	const Graph<int>& getMapGraph() const { return _graph; } 
private:
	Graph<int> _graph;
};

#endif

