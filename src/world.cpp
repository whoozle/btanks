#include "world.h"
#include "object.h"
#include "mrt/logger.h"
#include "sdlx/rect.h"

IMPLEMENT_SINGLETON(World, IWorld)


void IWorld::addObject(Object *o) {
	_objects.insert(o);
	LOG_DEBUG(("object %p added, objects: %d", (void*)o, _objects.size()));
}

void IWorld::render(sdlx::Surface &surface, const sdlx::Rect &viewport) {
	for(tObjectSet::iterator i = _objects.begin(); i != _objects.end(); ++i) {
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
	for(tObjectSet::iterator i = _objects.begin(); i != _objects.end(); ++i) {
		Object &o = **i;
		o.tick(dt);
		o._x += o._vx * dt;
		o._y += o._vy * dt;
		o._z += o._vz * dt;
	}
}
