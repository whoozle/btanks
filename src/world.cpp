#include "world.h"

IMPLEMENT_SINGLETON(World, IWorld)


void IWorld::addObject(Object *o) {
	_objects.insert(o);
}
