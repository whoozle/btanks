#ifndef __BT_WORLD_H__
#define __BT_WORLD_H__

#include "mrt/singleton.h"
#include <list>

struct ObjectInfo {
	float x, y, z;
};


class IWorld {
public:
	DECLARE_SINGLETON(IWorld);
private:
	typedef std::list<ObjectInfo> ObjectInfoList;
	ObjectInfoList _objects;
};

SINGLETON(World, IWorld);

#endif
