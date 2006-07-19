#include "world.h"
#include "object.h"
#include "world_map.h"

#include "mrt/exception.h"
#include "mrt/logger.h"
#include "sdlx/rect.h"

#include <math.h>

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

void IWorld::tick(WorldMap &map, const float dt) {
	//LOG_DEBUG(("tick dt = %f", dt));
	for(ObjectSet::iterator i = _objects.begin(); i != _objects.end(); ) {
		Object &o = **i;
		if (o.ttl > 0) {
			o.ttl -= dt;
			if (o.ttl <= 0) {
				//dead
				o.emit("death");
				o.ttl = 0;
			}
		}
		if (o.dead) {
			delete *i;
			_objects.erase(i++);
			continue;
		}
		
		o.tick(dt);
		float vx = o._vx, vy = o._vy, vz = o._vz;
		float len = sqrt(vx * vx + vy * vy + vz * vz);
		if (len == 0) {
			++i;
			continue;
		}

		//LOG_DEBUG(("im = %f", im));
		
		float dx = o.speed * vx / len * dt;
		float dy = o.speed * vy / len * dt;
		float dz = o.speed * vz / len * dt;

		float im = 1;
		if (o.piercing) {
			if (map.getImpassability(o, (int)(o._x + dx), (int)(o._y + dy), (int)(o._z + dz)) == 100) {
				o.emit("death"); //fixme
			}
		} else {
			im = 1 - map.getImpassability(o, (int)(o._x + dx), (int)(o._y + dy), (int)(o._z + dz)) / 100.0;
		}

		o._x += dx * im; 
		o._y += dy * im;
		o._z += dz * im;
		
		++i;
	}
}

const bool IWorld::exists(Object *o) const {
	return _objects.find(o) != _objects.end();
}

void IWorld::spawn(Object *src, Object *obj, const float dx, const float dy, const float dz, const float vx, const float vy, const float vz) {
	obj->_x = src->_x + dx;
	obj->_y = src->_y + dy;
	obj->_z = src->_z + dz;
	obj->_vx = vx;
	obj->_vy = vy;
	obj->_vz = vz;
	addObject(obj);
}
