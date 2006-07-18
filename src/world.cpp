#include "world.h"
#include "object.h"
#include "mrt/logger.h"
#include "sdlx/rect.h"
#include <math.h>
#include "mrt/exception.h"

IMPLEMENT_SINGLETON(World, IWorld)


void IWorld::addObject(Object *o) {
	if (o == NULL) 
		throw_ex(("adding NULL as world object is not allowed"));
		
	_objects.insert(o);
	LOG_DEBUG(("object %p added, objects: %d", (void*)o, _objects.size()));
}

const bool IWorld::getInfo(Object * po, float &x, float &y, float &z, float &vx, float &vy, float &vz) const {
	ObjectSet::const_iterator i = _objects.find(po);
	if (i == _objects.end())
		return false;
		
	const Object &o = **i;
	x = o._x;   y = o._y; 	z = o._z;
	vx = o._vx; vy = o._vy; vz = o._vz;

	float len = sqrt(vx * vx + vy * vy + vz * vz);
	if (len != 0) {
		vx /= len;
		vy /= len;
		vz /= len;
	}
	
	return true;
}


void IWorld::render(sdlx::Surface &surface, const sdlx::Rect &viewport) {
	for(ObjectSet::iterator i = _objects.begin(); i != _objects.end(); ++i) {
		Object &o = **i;
		sdlx::Rect r((int)o._x, (int)o._y, (int)o.w, (int) o.h);
		if (true /* r.in(viewport) */) {
			r.x -= viewport.x;
			r.y -= viewport.y;
			o.render(surface, r.x, r.y);
		}
	}
}

void IWorld::tick(const float dt) {
	//LOG_DEBUG(("tick dt = %f", dt));
	for(ObjectSet::iterator i = _objects.begin(); i != _objects.end(); ++i) {
		Object &o = **i;
		o.tick(dt);
		float vx = o._vx, vy = o._vy, vz = o._vz;
		float len = sqrt(vx * vx + vy * vy + vz * vz);
		if (len == 0)
			continue;
		
		o._x += o.speed * vx / len * dt;
		o._y += o.speed * vy / len * dt;
		o._z += o.speed * vz / len * dt;
	}
}
