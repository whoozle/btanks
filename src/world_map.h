#ifndef __BTANKS_WORLDMAP_H__
#define __BTANKS_WORLDMAP_H__

#include "v3.h"

class Object;
class WorldMap {
public:
	virtual const int getImpassability(Object &object, const v3<int> &pos) const = 0;
	virtual ~WorldMap() {}
};

#endif

