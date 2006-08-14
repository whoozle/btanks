#include "world.h"
#include "object.h"
#include "world_map.h"
#include "resource_manager.h"

#include "mrt/exception.h"
#include "mrt/logger.h"
#include "mrt/chunk.h"
#include "sdlx/rect.h"
#include "sdlx/surface.h"

#include <math.h>
#include <assert.h>
#include <limits>

#include "sdl_collide/SDL_collide.h"

IMPLEMENT_SINGLETON(World, IWorld)


void IWorld::addObject(Object *o, const v3<float> &pos) {
	static int last_id;
	if (o == NULL) 
		throw_ex(("adding NULL as world object is not allowed"));
	o->_id = ++last_id;
	
	assert (_id2obj.find(o->_id) == _id2obj.end());

	o->_position = pos;
	
	_objects.insert(o);
	_id2obj[o->_id] = o;

	assert(_id2obj.size() == _objects.size());
	//LOG_DEBUG(("object %d added, objects: %d", o->_id, _objects.size()));
}

const bool IWorld::getInfo(const Object * po, v3<float> &pos, v3<float> &vel) const {
	ObjectSet::const_iterator i = _objects.find((Object *)po);
	if (i == _objects.end())
		return false;
		
	const Object &o = **i;
	pos = o._position;
	vel = o._velocity;
	
	vel.normalize();
	
	return true;
}


void IWorld::render(sdlx::Surface &surface, const sdlx::Rect &viewport) {
	typedef std::multimap<const float, Object *> LayerMap;
	LayerMap layers;
	
	for(ObjectSet::iterator i = _objects.begin(); i != _objects.end(); ++i) {
		Object *o = *i;
		layers.insert(LayerMap::value_type(o->_position.z, o));
	}
	
	for(LayerMap::iterator i = layers.begin(); i != layers.end(); ++i) {
		Object &o = *i->second;
		sdlx::Rect r((int)o._position.x, (int)o._position.y, o.size.x, o.size.y);
		if (r.intersects(viewport)) {
			r.x -= viewport.x;
			r.y -= viewport.y;
			o.render(surface, r.x, r.y);
		}
	}
}

const float IWorld::getImpassability(Object *obj, const sdlx::Surface &surface, const v3<int> &position) const {
	sdlx::Rect my((int)position.x, (int)position.y,(int)obj->size.x, (int)obj->size.y);
	float im = 0;
	if (obj->_owner_id != 0 && _id2obj.find(obj->_owner_id) == _id2obj.end()) {
		obj->_owner_id = 0; //dead object.
	}
	
	for(ObjectSet::const_iterator i = _objects.begin(); i != _objects.end(); ++i) {
		Object *o = *i;
		if (o == obj) 
			continue;
		
		if ((obj->_owner_id != 0 && obj->_owner_id == o->_id) || (o->_owner_id != 0 && o->_owner_id == obj->_id)) 
			continue;
		
		sdlx::Rect other((int)o->_position.x, (int)o->_position.y,(int)o->size.x, (int)o->size.y);
		if (my.intersects(other)) {
	
			sdlx::Surface osurf;
			osurf.createRGB(other.w, other.h, 24, sdlx::Surface::Software | sdlx::Surface::Alpha );
			osurf.convertAlpha();
			osurf.fillRect(osurf.getSize(), SDL_MapRGBA(osurf.getPixelFormat(), 255, 0, 255, 255));
			o->render(osurf, 0, 0);
			
			v3<int> dpos = o->_position.convert<int>() - position;
			//LOG_DEBUG(("%s: %d %d", o->classname.c_str(), dpos.x, dpos.y));
			int r = SDL_CollidePixel(surface.getSDLSurface(), 0, 0, osurf.getSDLSurface(), dpos.x, dpos.y);
			if (r) {
				//LOG_DEBUG(("collision"));
				//LOG_DEBUG(("collision %s <-> %s", obj->classname.c_str(), o->classname.c_str()));
				o->emit("collision", obj);
				obj->emit("collision", o);

				if (im < o->impassability)
					im = o->impassability;
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
		if (o.isDead()) {
			_id2obj.erase((*i)->_id);
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
	
		osurf.createRGB(ow, oh, 24, sdlx::Surface::Software |  sdlx::Surface::Alpha);
		osurf.convertAlpha();
		osurf.fillRect(osurf.getSize(), SDL_MapRGBA(osurf.getPixelFormat(), 255, 0, 255, 255));
		o.render(osurf, 0, 0);
		
		//osurf.saveBMP("snapshot.bmp");
		
		float obj_im = getImpassability(*i, osurf, new_pos);
/*
		if (getImpassability(*i, osurf, o._position.convert<int>()) == 1.0 && obj_im == 1.0) {
			//obj_im = 0.1; //fix it.
		}
*/		//LOG_DEBUG(("obj_im = %f", obj_im));
		
		float map_im = 1;
		if (o.piercing) {
			if (map.getImpassability(osurf, new_pos) == 100) {
				o.emit("death"); //fixme
			}
		} else {
			map_im = 1 - map.getImpassability(osurf, new_pos) / 100.0;
/*			int old_im = map.getImpassability(osurf, o._position.convert<int>());
			if (old_im == 100 && map_im > 0) {
				map_im = 0.5; //special case, to work around animations causing object to "stuck" into solid objects.
			} 
*/		}

		if (o.isDead()) {
			_id2obj.erase((*i)->_id);
			delete *i;
			_objects.erase(i++);
			continue;
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
		//LOG_DEBUG(("%s is looking for %s. found: %s", obj->classname.c_str(), classname.c_str(), o->classname.c_str()));
		if (o->_id == obj->_id || o->classname != classname || o->_owner_id == obj->_id) 
			continue;
		
		float d = obj->_position.quick_distance(o->_position);
		if (d < distance) {
			distance = d;
			position = o->_position;
			velocity = o->_velocity;
			found = true;
		}
	}
	if (found) {
		position -= obj->_position;
	}
	return found;
}


const bool IWorld::exists(const Object *o) const {
	return _objects.find((Object *)o) != _objects.end();
}

const Object *IWorld::getObjectByID(const int id) const {
	ObjectMap::const_iterator i = _id2obj.find(id);
	if (i != _id2obj.end())
		return i->second;
	return NULL;
}


const Object* IWorld::spawn(Object *src, const std::string &classname, const std::string &animation, const v3<float> &dpos, const v3<float> &vel) {
	Object *obj = ResourceManager->createObject(classname, animation);
	assert(obj->_owner_id == 0);
	//LOG_DEBUG(("%s spawns %s", src->classname.c_str(), obj->classname.c_str()));
	obj->_owner_id = src->_id;
	obj->_velocity = vel;
	//LOG_DEBUG(("spawning %s, position = %f %f dPosition = %f:%f, velocity: %f %f", 
		//classname.c_str(), src->_position.x, src->_position.y, dpos.x, dpos.y, vel.x, vel.y));
	addObject(obj, src->_position + dpos);
	//LOG_DEBUG(("result: %f %f", obj->_position.x, obj->_position.y));
	return obj;
}

void IWorld::serialize(mrt::Serializator &s) const {
	s.add(_objects.size());
	for(ObjectSet::const_iterator i = _objects.begin(); i != _objects.end(); ++i) {
		const Object *o = *i;
		s.add(o->classname);
		s.add(o->animation);
		o->serialize(s);
	}
}

void IWorld::deserialize(const mrt::Serializator &s) {
	int size;
	s.get(size);
	while(size--) {
		std::string cn, an;
		s.get(cn);
		s.get(an);
		
		Object *ao = NULL;
		TRY {
			ao = ResourceManager->createObject(cn, an);
			LOG_DEBUG(("created ('%s', '%s')", cn.c_str(), an.c_str()));
			ao->deserialize(s);
			
			LOG_DEBUG(("deserialized %d: %s", ao->_id, ao->classname.c_str()));
			ObjectMap::iterator i;
			if ((i = _id2obj.find(ao->_id)) != _id2obj.end()) {
				Object *o = i->second;
				*o = *ao;
				delete ao; ao = NULL;
			} else {
				_id2obj[ao->_id] = ao;
				_objects.insert(ao);
				ao = NULL;
			}
		} CATCH("deserialize", { delete ao; ao = NULL; });
	}
	//LOG_DEBUG(("deserialization completed successfully"));
}
