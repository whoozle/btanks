#include "world.h"
#include "object.h"
#include "tmx/map.h"
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
	o->onSpawn();
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

void IWorld::getImpassabilityMatrix(Matrix<int> &matrix, const Object *src, const Object *dst) const {
	const v3<int> size = Map->getTileSize();
	
	Map->getImpassabilityMatrix(matrix);
	for(ObjectSet::const_iterator i = _objects.begin(); i != _objects.end(); ++i) {
		Object *o = *i;
		if (o == src || o == dst)
			continue;
		
		int im = (int)(o->impassability * 100);
		if (o->piercing || im == 0) 
			continue;
		if (im >= 100)
			im = -1;
		
		v3<int> p1, p2;
		p1 = o->_position.convert<int>();
		p2 = p1 + o->size - 1;
		
		for(int y = p1.y/IMap::pathfinding_step; y <= p2.y/IMap::pathfinding_step; ++y) 
			for(int x = p1.x/IMap::pathfinding_step; x <= p2.x/IMap::pathfinding_step; ++x) {
				//int old = matrix.get(y, x);
				//LOG_DEBUG(("%d %d = %d->%d", y, x, old, im));
				matrix.set(y, x, im);
			}
	}
}


void IWorld::tick(const float dt) {
	const IMap &map = *IMap::get_instance();
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
		
		o.updateState();
		o.tick(dt);
		
		{
			int f = o._follow;
			if (f != 0) {
				ObjectMap::const_iterator i = _id2obj.find(f);
				if (i != _id2obj.end()) {
					const Object *leader = i->second;
					//LOG_DEBUG(("following %d...", f));
					o._direction = leader->_direction;
					o._position = leader->_position + o._follow_position;
					o._velocity = leader->_velocity;
					o._old_velocity = leader->_old_velocity;
					o.setDirection(leader->getDirection());
				}
			}
		}
		
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
		
		dpos *= map_im * (1 - obj_im);
		if (o._distance > 0) {
			o._distance -= dpos.length();
		}
		o._position += dpos;
		++i;
	}
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

Object* IWorld::spawn(Object *src, const std::string &classname, const std::string &animation, const v3<float> &dpos, const v3<float> &vel) {
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

Object * IWorld::spawnGrouped(Object *src, const std::string &classname, const std::string &animation, const v3<float> &dpos, const GroupType type) {
	Object *obj = ResourceManager->createObject(classname, animation);
	assert(obj->_owner_id == 0);
	obj->_owner_id = src->_id;
	
	obj->_follow_position = dpos;
	switch(type) {
		case Centered:
			obj->_follow_position += (src->size.convert<float>() - obj->size.convert<float>())/2;
			break;
		case Fixed:
			break;
	}
	obj->follow(src);
	addObject(obj, obj->_position + obj->_follow_position);
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

//BIG PATHFINDING PART

typedef v3<int> vertex;
typedef std::deque<vertex> vertex_queue;

static void push(Matrix<int> &path, vertex_queue &buf, const vertex &vertex) {
	int w = path.get(vertex.y, vertex.x);
	if (w != -1 && w <= vertex.z) 
		return;
	path.set(vertex.y, vertex.x, vertex.z);
	buf.push_back(vertex);
}

static const bool pop(vertex_queue &buf, vertex &vertex) {
	if (buf.empty())
		return false;
	vertex = buf.front();
	buf.pop_front();
	return true;
}

inline static const int check(const Matrix<int> &imp, const vertex &v, const int dx, const int dy) {
	int w;
	if ((w = imp.get(v.y, v.x)) == -1)
		return -1;
	int r = w;
	
	if ((w = imp.get(v.y, v.x + dx)) == -1)
		return -1;
	if (w > r) r = w;

	if ((w = imp.get(v.y + dy, v.x)) == -1)
		return -1;
	if (w > r) r = w;

	if ((w = imp.get(v.y + dy, v.x + dx)) == -1)
		return -1;
	if (w > r) r = w;
	
	return r * 100 / 41;
}

const bool IWorld::getNearest(const Object *obj, const std::string &classname, v3<float> &position, v3<float> &velocity, Way * way) const {
	position.clear();
	velocity.clear();
	float distance = std::numeric_limits<float>::infinity();
	const Object *target = NULL;
	
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
			target = o;
		}
	}
	if (target == NULL) 
		return false;
	
	position -= obj->_position;
	if (way == NULL)
		return true;

	//finding shortest path.

	Matrix<int> imp, path;
	World->getImpassabilityMatrix(imp, obj, target);
	//LOG_DEBUG(("imp\n%s", imp.dump().c_str()));
	
	v3<int> src = obj->_position.convert<int>() / IMap::pathfinding_step;
	v3<int> dst = target->_position.convert<int>() / IMap::pathfinding_step;
	
	int w = imp.getWidth(), h = imp.getHeight();

	path.setSize(h, w, -1);
	path.useDefault(-1);
	
	vertex_queue buf;
	imp.set(src.y, src.x, 0);
	push(path, buf, vertex(src.x, src.y, 0));
	
	vertex v;
	while(pop(buf, v)) {
		int n = path.get(v.y, v.x);
		if (n == -1) 
			continue;
		int w = imp.get(v.y, v.x);
		//LOG_DEBUG(("get(%d, %d) = %d, %d", v.y, v.x, w, n));
		assert(w != -1);
		if (w == -1)
			continue;

		n += w + 1;
		
		if (imp.get(v.y + 1, v.x) != -1)
			push(path, buf, vertex(v.x, v.y + 1, n));
		if (imp.get(v.y - 1, v.x) != -1)
			push(path, buf, vertex(v.x, v.y - 1, n));
		if (imp.get(v.y, v.x + 1) != -1)
			push(path, buf, vertex(v.x + 1, v.y, n));
		if (imp.get(v.y, v.x - 1) != -1)
			push(path, buf, vertex(v.x - 1, v.y, n));

		if (check(imp, v, 1, 1) != -1)
			push(path, buf, vertex(v.x + 1, v.y + 1, n));
		if (check(imp, v, 1, -1) != -1)
			push(path, buf, vertex(v.x + 1, v.y - 1, n));
		if (check(imp, v, -1, 1) != -1)
			push(path, buf, vertex(v.x - 1, v.y + 1, n));
		if (check(imp, v, -1, -1) != -1)
			push(path, buf, vertex(v.x - 1, v.y - 1, n));
	}
	
	int len, n = path.get(dst.y, dst.x);
	len = n;
	
	if (n == -1) {
		/*
		imp.set(dst.y, dst.x, -99);
		imp.set(src.y, src.x, imp.get(src.y, src.x) - 100);
		LOG_DEBUG(("imp\n%s", imp.dump().c_str()));
		*/
		return true;
	}

	way->clear();
	int x = dst.x, y = dst.y;
	int i = -10;
	
	while ( x != src.x || y != src.y) {
		assert(imp.get(y, x) != -1);
		imp.set(y, x, i--);
		way->push_front(WayPoint(x, y, 0));
		int t = n;
		int x2 = x, y2 = y;

		int w = path.get(y + 1, x);
		if (w != -1 && w < t) {
			x2 = x; y2 = y + 1; t = w;
		}
		w = path.get(y - 1, x);
		if (w != -1 && w < t) {
			x2 = x; y2 = y - 1;	t = w;
		}
		w = path.get(y, x + 1);
		if (w != -1 && w < t) {
			x2 = x + 1; y2 = y; t = w;
		}
		w = path.get(y, x - 1);
		if (w != -1 && w < t) {
			x2 = x - 1; y2 = y; t = w;
		}
		//diagonals 
		w = path.get(y + 1, x + 1);
		if (w != -1 && w < t) {
			y2 = y + 1; x2 = x + 1; t = w;
		}
		w = path.get(y + 1, x - 1);
		if (w != -1 && w < t) {
			y2 = y + 1; x2 = x - 1; t = w;
		}
		w = path.get(y - 1, x + 1);
		if (w != -1 && w < t) {
			y2 = y - 1; x2 = x + 1; t = w;
		}
		w = path.get(y - 1, x - 1);
		if (w != -1 && w < t) {
			y2 = y - 1; x2 = x - 1; t = w;
		}
		assert(t != -1);
		
		x = x2; y = y2; n = t;
	}
	//result.push_front(WayPoint(x, y, 0));
	//LOG_DEBUG(("imp\n%s", imp.dump().c_str()));
	
	
	for(Way::iterator i = way->begin(); i != way->end(); ++i) {
		(*i) *= IMap::pathfinding_step;
	}
	
	//LOG_DEBUG(("getPath: length: %d, \n%s", len, result.dump().c_str()));
	return true;
}

