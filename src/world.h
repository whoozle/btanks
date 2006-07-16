#ifndef __BT_WORLD_H__
#define __BT_WORLD_H__

#include "mrt/singleton.h"
#include <set>

class Object;

class IWorld {
public:
	DECLARE_SINGLETON(IWorld);
	
	void addObject(Object *);
private:
	typedef std::set<Object *> tObjectSet;
	tObjectSet _objects;
};

SINGLETON(World, IWorld);

#endif
