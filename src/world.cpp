#include "world.h"
#include "object.h"
#include "world_map.h"

#include "mrt/exception.h"
#include "mrt/logger.h"
#include "sdlx/rect.h"
#include "sdlx/surface.h"

#include <math.h>
#include <assert.h>
#include <limits>

#include "sdl_collide/SDL_collide.h"

IMPLEMENT_SINGLETON(World, IWorld)


void IWorld::addObject(Object *o, const v3<float> &pos) {
	if (o == NULL) 
		throw_ex(("adding NULL as world object is not allowed"));
	o->_position = pos;
		
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
		sdlx::Rect r((int)o._position.x, (int)o._position.y, o.size.x, o.size.y);
		if (true /* r.in(viewport) */) {
			r.x -= viewport.x;
			r.y -= viewport.y;
			o.render(surface, r.x, r.y);
		}
	}
}

const float IWorld::getImpassability(Object &obj, const sdlx::Surface &surface, const v3<int> &position) const {
	sdlx::Rect my((int)position.x, (int)position.y,(int)obj.size.x, (int)obj.size.y);
	float im = 0;
	
	for(ObjectSet::iterator i = _objects.begin(); i != _objects.end(); ++i) {
		Object &o = **i;
		if (&o == &obj) 
			continue;
		
		sdlx::Rect other((int)o._position.x, (int)o._position.y,(int)o.size.x, (int)o.size.y);
		if (my.intersects(other)) {
	
			sdlx::Surface osurf;
			osurf.createRGB(other.w, other.h, 24, sdlx::Surface::Software);
			osurf.convertAlpha();
			osurf.fillRect(osurf.getSize(), SDL_MapRGBA(osurf.getPixelFormat(), 0,0,0,255));
			o.render(osurf, 0, 0);
			
			//v3<int> dpos = o._position.convert<int>() - position;
			v3<int> dpos = position - o._position.convert<int>();
			LOG_DEBUG(("%s: %d %d", o.classname.c_str(), dpos.x, dpos.y));
			int r = SDL_CollidePixel(osurf.getSDLSurface(), dpos.x, dpos.y, surface.getSDLSurface(), 0, 0);
			if (r) {
				//LOG_DEBUG(("collision"));
				LOG_DEBUG(("collision %s <-> %s", obj.classname.c_str(), o.classname.c_str()));
				o.emit("collision", &obj);
				obj.emit("collision", &o);

				if (im < o.impassability)
					im = o.impassability;
			}
		}
	}
	
	return im;
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
		o._old_velocity = vel;
		
		if (len == 0) {
			++i;
			continue;
		}
		o._direction = o._velocity;

		//LOG_DEBUG(("im = %f", im));
		v3<float> dpos = o.speed * vel * dt;
/*		
		float dx = o.speed * vx / len * dt;
		float dy = o.speed * vy / len * dt;
		float dz = o.speed * vz / len * dt;
*/
		v3<int> new_pos((o._position + dpos).convert<int>());

		int ow = o.size.x;
		int oh = o.size.y; 

		sdlx::Surface osurf;
		
		assert(ow != 0 && oh != 0);
	
		osurf.createRGB(ow, oh, 24, sdlx::Surface::Software);
		osurf.convertAlpha();
		osurf.fillRect(osurf.getSize(), SDL_MapRGBA(osurf.getPixelFormat(), 0,0,0,255));
		o.render(osurf, 0, 0);
		//s.saveBMP("snapshot.bmp");
		
		float obj_im = getImpassability(o, osurf, new_pos);
		//LOG_DEBUG(("obj_im = %f", obj_im));
		
		float map_im = 1;
		if (o.piercing) {
			if (map.getImpassability(osurf, new_pos) == 100) {
				o.emit("death"); //fixme
			}
		} else {
			map_im = 1 - map.getImpassability(osurf, new_pos) / 100.0;
		}
		o._position += dpos * map_im * (1 - obj_im);
		++i;
	}
}

const bool IWorld::getNearest(const Object *obj, const std::string &classname, v3<float> &position, v3<float> &velocity) const {
	bool found = false;
	
	position.clear();
	velocity.clear();
	float distance = std::numeric_limits<float>::infinity();
	
	for(ObjectSet::const_iterator i = _objects.begin(); i != _objects.end(); ++i) {
		const Object *o = *i;
		if (o == obj || o->classname != classname) 
			continue;
		float d = obj->_position.quick_distance(o->_position);
		if (d < distance) {
			distance = d;
			position = o->_position;
			velocity = o->_velocity;
			found = true;
		}
	}
	return found;
}


const bool IWorld::exists(Object *o) const {
	return _objects.find(o) != _objects.end();
}

void IWorld::spawn(Object *src, Object *obj, const v3<float> &dpos, const v3<float> &vel) {
	obj->_velocity = vel;
	addObject(obj, src->_position + dpos);
}
