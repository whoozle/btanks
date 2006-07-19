#ifndef __BTANKS_WORLDMAP_H__
#define __BTANKS_WORLDMAP_H__

class Object;
class WorldMap {
public:
	virtual const int getImpassability(Object &object, const int x, const int y, const int z) const = 0;
	virtual ~WorldMap() {}
};

#endif

