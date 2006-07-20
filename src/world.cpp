#include "world.h"
#include "object.h"
#include "world_map.h"

#include "mrt/exception.h"
#include "mrt/logger.h"
#include "sdlx/rect.h"
#include "sdlx/surface.h"

#include <math.h>
#include <assert.h>

IMPLEMENT_SINGLETON(World, IWorld)


void IWorld::addObject(Object *o) {
	if (o == NULL) 
		throw_ex(("adding NULL as world object is not allowed"));
		
	_objects.insert(o);
	LOG_DEBUG(("object %p added, objects: %d", (void*)o, _objects.size()));
}

const bool IWorld::getInfo(Object * po, v3<float> &pos, v3<float> &vel) const {
	ObjectSet::const_iterator i = _objects.find(po);
	if (i == _objects.end())
		return false;
		
	const Object &o = **i;
	pos = o._position;
	vel = o._velocity;
	
	vel.normalize();
	
	return true;
}


void IWorld::render(sdlx::Surface &surface, const sdlx::Rect &viewport) {
	for(ObjectSet::iterator i = _objects.begin(); i != _objects.end(); ++i) {
		Object &o = **i;
		sdlx::Rect r((int)o._position.x, (int)o._position.y, (int)o.w, (int) o.h);
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
		
		v3<float> vel = o._velocity;
		float len = vel.normalize();
		if (len == 0) {
			++i;
			continue;
		}
		o._old_velocity = o._velocity;

		//LOG_DEBUG(("im = %f", im));
		v3<float> dpos = o.speed * vel * dt;
/*		
		float dx = o.speed * vx / len * dt;
		float dy = o.speed * vy / len * dt;
		float dz = o.speed * vz / len * dt;
*/
		v3<int> new_pos((o._position + dpos).convert<int>());

		int ow = (int)o.w;
		int oh = (int)o.h; 

		sdlx::Surface osurf;
		
		assert(ow != 0 && oh != 0);
	
		osurf.createRGB(ow, oh, 24, sdlx::Surface::Software);
		osurf.convertAlpha();
		osurf.fillRect(osurf.getSize(), SDL_MapRGBA(osurf.getPixelFormat(), 0,0,0,255));
		o.render(osurf, 0, 0);
		//s.saveBMP("snapshot.bmp");
		
		float im = 1;
		if (o.piercing) {
			if (map.getImpassability(osurf, new_pos) == 100) {
				o.emit("death"); //fixme
			}
		} else {
			im = 1 - map.getImpassability(osurf, new_pos) / 100.0;
		}
		o._position += dpos * im;
		++i;
	}
}

const bool IWorld::exists(Object *o) const {
	return _objects.find(o) != _objects.end();
}

void IWorld::spawn(Object *src, Object *obj, const v3<float> &dpos, const v3<float> &vel) {
	obj->_position = src->_position + dpos;
	obj->_velocity = vel;
	addObject(obj);
}
