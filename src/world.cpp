
/* Battle Tanks Game
 * Copyright (C) 2006 Battle Tanks team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "world.h"
#include "object.h"
#include "tmx/map.h"
#include "resource_manager.h"
#include "player_manager.h"
#include "config.h"
#include "utils.h"

#include "mrt/exception.h"
#include "mrt/logger.h"
#include "mrt/chunk.h"
#include "sdlx/rect.h"
#include "sdlx/surface.h"

#include <math.h>
#include <assert.h>
#include <limits>


IMPLEMENT_SINGLETON(World, IWorld)

void IWorld::clear() {
	std::for_each(_objects.begin(), _objects.end(), delete_ptr<Object *>());
	_objects.clear();
	_id2obj.clear();
	_last_id = 0;
	_safe_mode = false;
}

IWorld::IWorld() : _last_id(0), _safe_mode(false) {}

IWorld::~IWorld() {
	clear();
}

void IWorld::setSafeMode(const bool safe_mode) {
	_safe_mode = safe_mode;
	LOG_DEBUG(("set safe mode to %s", _safe_mode?"true":"false"));
}


void IWorld::addObject(Object *o, const v3<float> &pos, const int id) {
	if (o == NULL) 
		throw_ex(("adding NULL as world object is not allowed"));
	o->_id = (id >= 0)?id:++_last_id;
	
	assert (_id2obj.find(o->_id) == _id2obj.end());
	assert (_objects.find(o) == _objects.end());

	float oz = o->_position.z;
	o->_position = pos;
	if (pos.z != 0) {
		LOG_DEBUG(("overriding z(%g) for object '%s'", pos.z, o->classname.c_str()));
	} else {
		o->_position.z = oz; //restore original value
		//LOG_DEBUG(("using default z(%g) for object '%s'", oz, o->classname.c_str()));
	}
	
	_objects.insert(o);
	_id2obj[o->_id] = o;

	assert(_id2obj.size() == _objects.size());
	o->onSpawn();
	o->need_sync = true;
	//LOG_DEBUG(("object %d added, objects: %d", o->_id, _objects.size()));
}

void IWorld::render(sdlx::Surface &surface, const sdlx::Rect&src, const sdlx::Rect &dst) {
	surface.setClipRect(dst);
	
	typedef std::multimap<const float, Object *> LayerMap;
	LayerMap layers;
	const IMap &map = *Map.get_const();
	
	for(ObjectSet::iterator i = _objects.begin(); i != _objects.end(); ++i) {
		Object *o = *i;
		if (o->isDead())
			continue;
		layers.insert(LayerMap::value_type(o->_position.z, o));
	}
	int z1 = -1001;
	for(LayerMap::iterator i = layers.begin(); i != layers.end(); ++i) {
		if (i->second->isDead())
			continue;
		
		int z2 = (int)i->first;
		//LOG_DEBUG(("world::render(%d, %d)", z1, z2));
		if (z1 != z2) {
			//LOG_DEBUG(("calling map::render(%d, %d)", z1, z2));
			map.render(surface, src, dst, z1, z2);
		}
		z1 = z2;
		Object &o = *i->second;
		sdlx::Rect r((int)o._position.x, (int)o._position.y, (int)o.size.x, (int)o.size.y);
		if (r.intersects(src)) {
			//LOG_DEBUG(("rendering %s with z = %g", o.classname.c_str(), o._position.z));
			o.render(surface, r.x - src.x + dst.x, r.y - src.y + dst.y);
		}
		
		GET_CONFIG_VALUE("engine.show-waypoints", bool, show_waypoints, false);
		if (show_waypoints) {
			const sdlx::Surface * wp_surface = ResourceManager->getSurface("waypoint16x16.png");
			const Way & way = o.getWay();
			for(Way::const_iterator wi = way.begin(); wi != way.end(); ++wi) {
				const v3<int> &wp = *wi;
				surface.copyFrom(*wp_surface, 
					wp.x - src.x + dst.x + (int)(o.size.x/2) - 8, wp.y - src.y + dst.y + (int)(o.size.y/2) - 8);
			}
		}
	}
	map.render(surface, src, dst, z1, 1000);
	
	surface.resetClipRect();
}

const bool IWorld::collides(Object *obj, const v3<int> &position, Object *o, const bool probe) const {
		if (o == obj || obj->impassability == 0 || o->impassability == 0 || 
			(obj->piercing && o->pierceable) || (obj->pierceable && o->piercing) ||
			o->isDead() || obj->isDead() ) {
			return false;
		}
			
		//skip owner and grouped-leader.
		if (
			(obj->_owner_id != 0 && (obj->_owner_id == o->_id || obj->_owner_id == o->_owner_id )) || 
			(o->_owner_id != 0 && o->_owner_id == obj->_id) ||
			(obj->_follow != 0 && obj->_follow == o->_id) || 
			(o->_follow != 0 && o->_follow == obj->_id) 
		) {
			return false;
		}

		const int id1 = obj->_id;
		const int id2 = o->_id;
		
		CollisionMap::key_type key = (id1 < id2) ? CollisionMap::key_type(id1, id2): CollisionMap::key_type(id2, id1);
		CollisionMap::iterator i = _collision_map.find(key);
		if (i != _collision_map.end()) {
			//LOG_DEBUG(("skipped collision detection for %s<->%s", obj->classname.c_str(), o->classname.c_str()));
			return i->second;
		}

		
		v3<int> dpos = o->_position.convert<int>() - position;
		//LOG_DEBUG(("%s: %d %d", o->classname.c_str(), dpos.x, dpos.y));
		const bool collides = obj->collides(o, dpos.x, dpos.y);
		//LOG_DEBUG(("collision %s <-> %s: %s", obj->classname.c_str(), o->classname.c_str(), collides?"true":"false"));
		if (!collides) {
			_collision_map.insert(CollisionMap::value_type(key, false));
			return false;
		}

		if (!probe && (o->impassability < 0 || o->impassability >= 1.0)) { //do not generate collision event if impassability != 1 and impassability != -1
			//LOG_DEBUG(("collision"));
			//LOG_DEBUG(("collision %s <-> %s", obj->classname.c_str(), o->classname.c_str()));
			o->emit("collision", obj);
			obj->emit("collision", o);
			
			if (o->isDead() && o->classname == "player") {
				PlayerManager->onPlayerDeath(o, obj);
			}

			if (obj->isDead() && obj->classname == "player") {
				PlayerManager->onPlayerDeath(obj, o);
			}
			
			if (o->isDead() || obj->isDead() || obj->impassability == 0 || o->impassability == 0) {
				_collision_map.insert(CollisionMap::value_type(key, false));
				return false; // no effect.
			}
			_collision_map.insert(CollisionMap::value_type(key, true));
		}
		//LOG_DEBUG(("collision %s <-> %s: %s", obj->classname.c_str(), o->classname.c_str(), collides?"true":"false"));
		
		return true;
}


const float IWorld::getImpassability(Object *obj, const v3<int> &position, const Object **collided_with, const bool probe) const {
	if (obj->impassability == 0) {
		if (collided_with != NULL)
			*collided_with = NULL;
		return 0;
	}
	
	float im = 0;
	const Object *result = NULL;
	sdlx::Rect my((int)position.x, (int)position.y,(int)obj->size.x, (int)obj->size.y);
	
	for(ObjectSet::const_iterator i = _objects.begin(); i != _objects.end(); ++i) {
		Object *o = *i;

		sdlx::Rect other((int)o->_position.x, (int)o->_position.y,(int)o->size.x, (int)o->size.y);
		if (!my.intersects(other)) 
			continue;

		if (!collides(obj, position, o, probe)) 
			continue;
		
		if (o->impassability > im) {
			im = o->impassability;
			result = o;
		}

	}
	if (collided_with != NULL)
		*collided_with = result;
	
	return im;
}

void IWorld::getImpassability2(float &old_pos_im, float &new_pos_im, Object *obj, const v3<int> &new_position, const Object **old_pos_collided_with) const {
	old_pos_im = 0;
	new_pos_im = 0;

	if (obj->impassability == 0) {
		if (old_pos_collided_with != NULL)
			*old_pos_collided_with = NULL;
		return;
	}
	
	v3<int> old_position = 	obj->_position.convert<int>();
	const Object *result = NULL;
	sdlx::Rect my_new((int)new_position.x, (int)new_position.y,(int)obj->size.x, (int)obj->size.y);
	sdlx::Rect my_old((int)obj->_position.x, (int)obj->_position.y,(int)obj->size.x, (int)obj->size.y);
	
	for(ObjectSet::const_iterator i = _objects.begin(); i != _objects.end(); ++i) {
		Object *o = *i;

		sdlx::Rect other((int)o->_position.x, (int)o->_position.y,(int)o->size.x, (int)o->size.y);
		if (!my_old.intersects(other) && !my_new.intersects(other)) 
			continue;

	//old position collisions
		if (collides(obj, old_position, o)) {
			if (o->impassability > old_pos_im) {
				old_pos_im = o->impassability;
				result = o;
			}
		}
	//new position collisions
		if (collides(obj, new_position, o)) {
			if (o->impassability > new_pos_im) {
				new_pos_im = o->impassability;
			}
		}

	}
	if (old_pos_collided_with != NULL)
		*old_pos_collided_with = result;
}


void IWorld::getImpassabilityMatrix(Matrix<int> &matrix, const Object *src, const Object *dst) const {
	const v3<int> size = Map->getTileSize();
	
	Map->getImpassabilityMatrix(matrix);
	for(ObjectSet::const_iterator i = _objects.begin(); i != _objects.end(); ++i) {
		Object *o = *i;
		if (o == src || o == dst || o->impassability <= 0 || o->piercing)
			continue;
		
		int im = (int)(o->impassability * 100);
		if (im >= 100)
			im = -1;
		
/*
		v3<int> p1, p2;
		p1 = o->_position.convert<int>();
		p2 = (o->_position + o->size - 1).convert<int>();
		
		for(int y = p1.y/IMap::pathfinding_step; y <= p2.y/IMap::pathfinding_step; ++y) 
			for(int x = p1.x/IMap::pathfinding_step; x <= p2.x/IMap::pathfinding_step; ++x) {
				int old = matrix.get(y, x);
				//LOG_DEBUG(("%d %d = %d->%d", y, x, old, im));
				if (old >= 0 && im > old || im == -1) 
					matrix.set(y, x, im);
			}
*/
		v3<int> p = (o->_position + o->size/2).convert<int>();
		int y = p.y / IMap::pathfinding_step;
		int x = p.x / IMap::pathfinding_step;
		int old = matrix.get(y, x);
		if ((old >= 0 && im > old) || im == -1) 
			matrix.set(y, x, im);
		
	}
	//LOG_DEBUG(("projected objects:\n%s", matrix.dump().c_str()));
}

void IWorld::tick(Object &o, const float dt) {
	if (o.isDead()) 
		return;
	//LOG_DEBUG(("tick object %p: %d: %s", (void *)&o, o.getID(), o.classname.c_str()));

	GET_CONFIG_VALUE("engine.max-time-slice", float, max_dt, 0.025);
	if (max_dt <= 0) 
		throw_ex(("invalid max-time-slice value %g", max_dt));

	if (dt > max_dt) {
		float dt2 = dt;
		while(dt2 > max_dt) {
			tick(o, max_dt);
			dt2 -= max_dt;
		}
		if (dt2 > 0) 
			tick(o, dt2);
		return;
	}

	if (dt < -max_dt) {
		float dt2 = dt;
		while(dt2 < -max_dt) {
			tick(o, -max_dt);
			dt2 += max_dt;
		}
		if (dt2 < 0) 
			tick(o, dt2);
		return;
	}
	
	const IMap &map = *IMap::get_instance();
	v3<int> map_size = map.getSize();
	
	if (o.ttl > 0) {
		o.ttl -= dt;
		if (o.ttl <= 0) {
			//dead
			o.emit("death");
			o.ttl = 0;
		}
	}
	if (o.isDead()) 
		return;
		
	v3<float> old_vel = o._velocity;

	TRY { 
		o.calculate(dt);
	} CATCH("calling o.calculate", throw;)

	GET_CONFIG_VALUE("engine.disable-z-velocity", bool, disable_z, true);
	if (disable_z)
		o._velocity.z = 0; //hack to prevent objects moving up/down.
		
	TRY { 
		o.tick(dt);
	} CATCH("calling o.tick", throw;)

	if (disable_z)
		o._velocity.z = 0; 
		
	{
		int f = o._follow;
		if (f != 0) {
			ObjectMap::const_iterator o_i = _id2obj.find(f);
			if (o_i != _id2obj.end()) {
				const Object *leader = o_i->second;
				//LOG_DEBUG(("following %d...", f));
				float z = o._position.z;
				o.speed = leader->speed;
				
				o._position = leader->_position + o._follow_position;
				o._position.z = z;
				o._velocity = leader->_velocity;
				return;
			} else {
				LOG_WARN(("leader for object %d is dead. (leader-id:%d)", o._id, f));
				o._follow = 0;
				o.emit("death", NULL);
			}
		}
	}
		
	if (o.speed == 0) {
		o._idle_time += dt;
		if (o.impassability < 0) {
			getImpassability(&o, o._position.convert<int>());
		}
		return;
	}
		
	float len = o._velocity.normalize();
		
	if (len == 0) {
		if (o._moving_time > 0) {
		//use inertia only if object co-directional with velocity. fixme
			if (!o.rotating())
				o._velocity_fadeout = old_vel;
		}
		o._moving_time = 0;
		o._idle_time += dt;
		if (o._velocity_fadeout.is0()) 
			return;
	} else {
		o._idle_time = 0;
		o._moving_time += dt;
		o._direction = o._velocity;
	}
	
	GET_CONFIG_VALUE("engine.mass-acceleration-divisor", float, ac_div, 1000.0);

	const float ac_t = o.mass / ac_div;
	if (o.mass > 0 && o._moving_time < ac_t) {
		o._velocity *= o._moving_time / ac_t * o._moving_time / ac_t;
	}
	o._velocity += o._velocity_fadeout;

	//LOG_DEBUG(("im = %f", im));
	v3<float> dpos = o.speed * o._velocity * dt;
	v3<int> new_pos((o._position + dpos).convert<int>());
	v3<int> old_pos = o._position.convert<int>();

	//osurf.saveBMP("snapshot.bmp");
	const Object *stuck_in = NULL;
	v3<int> stuck_map_pos;
	bool stuck = false;

	float map_im = 0, obj_im_now = 0, obj_im = 0;
	int attempt;
	
	int save_dir = o.getDirection();
	int dirs = o.getDirectionsNumber();
	bool hidden_attempt[3] = { false, false, false };
	bool hidden;
	const std::string outline_animation = o.registered_name + "-outline";
	const bool has_outline = ResourceManager->hasAnimation(outline_animation);
	
	for(attempt =0; attempt < 3; ++attempt) {
		v3<int> pos = new_pos;
		v3<float> v(o._velocity);
		if (attempt == 1) {
			v.x = 0;
			pos.x = old_pos.x; 
			v.normalize();
			o.setDirection(v.getDirection(dirs) - 1);
		} else if (attempt == 2) {
			v.y = 0;
			pos.y = old_pos.y;
			v.normalize();
			o.setDirection(v.getDirection(dirs) - 1);
		}
		
		map_im = map.getImpassability(&o, pos, NULL, has_outline?(hidden_attempt + attempt):NULL) / 100.0;
		getImpassability2(obj_im_now, obj_im, &o, pos, &stuck_in);

		if ((map_im < 1.0 && obj_im < 1.0) || o.piercing)
			break;

		stuck = map.getImpassability(&o, old_pos, &stuck_map_pos) == 100 || obj_im_now >= 1.0;
	}
	
	if (attempt == 1) {
		o._velocity.x = 0;
		len = o._velocity.normalize();
		hidden = hidden_attempt[1];
	} else if (attempt == 2) {
		o._velocity.y = 0;
		len = o._velocity.normalize();
		hidden = hidden_attempt[2];
	} else {
		o.setDirection(save_dir);
		hidden = hidden_attempt[0];
	}
	
	if (has_outline) {
		if (hidden) {
			if (has_outline && !o.has("_outline")) {
				LOG_DEBUG(("%d:%s:%s: adding outline", o._id, o.classname.c_str(), o.animation.c_str()));
				o.add("_outline", o.spawnGrouped("outline", outline_animation, v3<float>::empty, Centered));
			}
		//LOG_DEBUG(("%d:%s:%s: whoaaa!!! i'm in domik", o._id, o.classname.c_str(), o.animation.c_str()));
		} else {
			if (o.has("_outline")) {
				LOG_DEBUG(("%d:%s:%s: removing outline", o._id, o.classname.c_str(), o.animation.c_str()));
				o.remove("_outline");
			}
		}
	}

	dpos = o.speed * o._velocity * dt;
	new_pos = (o._position + dpos).convert<int>();

	if (o.piercing) {
		if (obj_im_now > 0 && obj_im_now < 1.0)
			obj_im_now = 0;
		if (map_im >= 1.0) {
			o._position += dpos;
			o.emit("collision", NULL); //fixme: emit collisions with map from map::getImpassability
			o._position -= dpos;
		} else map_im = 0;
	}
	

	if (obj_im == 1.0 || map_im == 1.0) {
		if (stuck) {
			v3<float> allowed_velocity;
			v3<float> object_center = o._position + o.size / 2;
			if (map_im == 1.0) {
				//LOG_DEBUG(("stuck: object: %g %g, map: %d %d", o._position.x, o._position.y, stuck_map_pos.x, stuck_map_pos.y));
				allowed_velocity = object_center - stuck_map_pos.convert<float>();
				allowed_velocity.z = 0;
				//LOG_DEBUG(("allowed-velocity: %g %g", allowed_velocity.x, allowed_velocity.y));
				if (allowed_velocity.same_sign(o._velocity) || allowed_velocity.is0()) {
					map_im = 0.5;
				}
				goto skip_collision;
			} else if (obj_im == 1.0) {
				assert(stuck_in != NULL);
				allowed_velocity = object_center - (stuck_in->_position + stuck_in->size/2);
				allowed_velocity.z = 0;
				//LOG_DEBUG(("allowed: %g %g", allowed_velocity.x, allowed_velocity.y));
				if (allowed_velocity.same_sign(o._velocity) || allowed_velocity.is0()) {
					//LOG_DEBUG(("stuck in object: %s, trespassing allowed!", stuck_in->classname.c_str()));
					obj_im = map_im;
					goto skip_collision;
				}
			}
		}
		//LOG_DEBUG(("bang!"));
		GET_CONFIG_VALUE("engine.bounce-velocity-multiplier", float, bvm, 0.5);
		
		o._velocity_fadeout = -bvm * o._velocity;
		o._velocity.clear();
		
		o._moving_time = 0;
	}

skip_collision:

	if (o.isDead())
		return;
/*
	if (o.piercing) {
		LOG_DEBUG(("%s *** %g,%g", o.dump().c_str(), map_im, obj_im));
	}
*/	
	dpos *= (1 - map_im) * (1 - obj_im);

	if (o.classname == "player") {
		if (o._position.x + dpos.x < 0 || o._position.x + dpos.x + o.size.x >= map_size.x)
			dpos.x = 0;

		if (o._position.y + dpos.y < 0 || o._position.y + dpos.y + o.size.y >= map_size.y)
			dpos.y = 0;
		
	} else {
		if (o._position.x + dpos.x < -o.size.x || o._position.x + dpos.x >= map_size.x)
			dpos.x = 0;

		if (o._position.y + dpos.y < -o.size.y || o._position.y + dpos.y >= map_size.y)
			dpos.y = 0;
	
	}
	o._position += dpos;
	
	
	GET_CONFIG_VALUE("engine.velocity-fadeout-multiplier", float, vf_m, 0.9);
	
	o._velocity_fadeout *= vf_m;
	//LOG_DEBUG(("vfadeout: %g %g", o._velocity_fadeout.x, o._velocity_fadeout.y));
	if (o._velocity_fadeout.quick_length() < 0.1) {
		o._velocity_fadeout.clear();
	}
}


void IWorld::tick(const float dt) {
	//LOG_DEBUG(("tick dt = %f", dt));
	_collision_map.clear();
	tick(_objects, dt);
}

void IWorld::tick(ObjectSet &objects, const float dt) {
	GET_CONFIG_VALUE("engine.max-time-slice", float, max_dt, 0.025);
	if (max_dt <= 0) 
		throw_ex(("invalid max-time-slice value %g", max_dt));

	if (dt > max_dt) {
		float dt2 = dt;
		while(dt2 > max_dt) {
			tick(objects, max_dt);
			dt2 -= max_dt;
		}
		if (dt2 > 0) 
			tick(objects, dt2);
		return;
	}

	if (dt < -max_dt) {
		float dt2 = dt;
		while(dt2 < -max_dt) {
			tick(objects, -max_dt);
			dt2 += max_dt;
		}
		if (dt2 < 0) 
			tick(objects, dt2);
		return;
	}

	for(ObjectSet::iterator i = objects.begin(); i != objects.end(); ) {
		Object *o = *i;
		assert(o != NULL);
		TRY {
			tick(*o, dt);
		} CATCH(mrt::formatString("tick for object[%p] %d:%s", (void *)o, o->getID(), o->classname.c_str()).c_str(), throw;);
		if (o->isDead()) { //fixme
			if (_safe_mode == false) {
				//LOG_DEBUG(("object %d:%s is dead. cleaning up. (global map: %s)", o->getID(), o->classname.c_str(), &objects == &_objects?"true":"false" ));
				ObjectMap::iterator m = _id2obj.find(o->_id);
				assert(m != _id2obj.end());
				assert(o == m->second);
				_id2obj.erase(m);
			
				//implement more smart way to fix it.
				if (&objects != &_objects)
					_objects.erase(o);
				objects.erase(i++);
				assert(_id2obj.size() == _objects.size());

				delete o;
				continue;
			}
		} 
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

Object *IWorld::getObjectByID(const int id) {
	ObjectMap::iterator i = _id2obj.find(id);
	if (i != _id2obj.end())
		return i->second;
	return NULL;
}


Object* IWorld::spawn(Object *src, const std::string &classname, const std::string &animation, const v3<float> &dpos, const v3<float> &vel) {
	Object *obj = ResourceManager->createObject(classname, animation);
	assert(obj->_owner_id == 0);
	//LOG_DEBUG(("%s spawns %s", src->classname.c_str(), obj->classname.c_str()));
	obj->_spawned_by = obj->_owner_id = src->_id;
	obj->_velocity = vel;
	
	//LOG_DEBUG(("spawning %s, position = %g %g dPosition = %g:%g, velocity: %g %g", 
	//	classname.c_str(), src->_position.x, src->_position.y, dpos.x, dpos.y, vel.x, vel.y));
	v3<float> pos = src->_position + (src->size / 2)+ dpos - (obj->size / 2);
	pos.z = 0;
	if (dpos.z != 0) {
		pos.z = dpos.z;
	}
	addObject(obj, pos);
	//LOG_DEBUG(("result: %f %f", obj->_position.x, obj->_position.y));
	return obj;
}

Object * IWorld::spawnGrouped(Object *src, const std::string &classname, const std::string &animation, const v3<float> &dpos, const GroupType type) {
	Object *obj = ResourceManager->createObject(classname, animation);
	assert(obj->_owner_id == 0);
	obj->_spawned_by = obj->_owner_id = src->_id;

	
	obj->_follow_position = dpos;
	switch(type) {
		case Centered:
			obj->_follow_position += (src->size - obj->size)/2;
			break;
		case Fixed:
			break;
	}
	obj->follow(src);

	v3<float> pos = obj->_position + obj->_follow_position;
	pos.z = 0;
	if (dpos.z != 0) {
		pos.z = dpos.z;
	}
	
	addObject(obj, pos);
	return obj;
}

void IWorld::serializeObject(mrt::Serializator &s, const Object *o) const {
	s.add(o->_id);
	s.add(o->registered_name);
	s.add(o->animation);
	o->serialize(s);
}


void IWorld::serialize(mrt::Serializator &s) const {
	s.add(_last_id);
	s.add((unsigned int)_id2obj.size());
	for(ObjectMap::const_reverse_iterator i = _id2obj.rbegin(); i != _id2obj.rend(); ++i) {
		const Object *o = i->second;
		serializeObject(s, o);
	}
}

Object * IWorld::deserializeObject(const mrt::Serializator &s) {
	int id;
	std::string rn, an;
	Object *ao = NULL, *result;
	TRY {
		s.get(id);
		s.get(rn);
		s.get(an);
		
		{
			ObjectMap::iterator i = _id2obj.find(id);
			if (i != _id2obj.end()) {
				Object *o = i->second;
				if (rn == o->registered_name) {
					o->deserialize(s);
					result = o;
				} else {
					//wrong classtype and maybe storage class
					_objects.erase(o);
					delete o;
					result = ao = ResourceManager->createObject(rn, an);
					//LOG_DEBUG(("created ('%s', '%s')", rn.c_str(), an.c_str()));
					ao->deserialize(s);
						
					i->second = ao;
					_objects.insert(ao);
					ao = NULL;
					assert(_id2obj.size() == _objects.size());
				}
			} else {
				//new object.
				result = ao = ResourceManager->createObject(rn, an);
				//LOG_DEBUG(("created ('%s', '%s')", rn.c_str(), an.c_str()));
				ao->deserialize(s);
				
				_id2obj[id] = ao;
				_objects.insert(ao);
				ao = NULL;
				assert(_id2obj.size() == _objects.size());
			}

			//LOG_DEBUG(("deserialized %d: %s", ao->_id, ao->classname.c_str()));
		}
	} CATCH(mrt::formatString("deserializeObject('%d:%s:%s')", id, rn.c_str(), an.c_str()).c_str(), { delete ao; throw; })
	assert(result != NULL);
	LOG_DEBUG(("deserialized object: %d:%s:%s", id, rn.c_str(), an.c_str()));
	return result;
}

void IWorld::cropObjects(const std::set<int> &ids) {
	for(ObjectMap::iterator i = _id2obj.begin(); i != _id2obj.end(); /*haha*/ ) {
		if (ids.find(i->first) == ids.end()) {
			delete i->second;
			_objects.erase(i->second);
			_id2obj.erase(i++);
		} else ++i;
	}
	assert(_id2obj.size() == _objects.size());
}

void IWorld::deserialize(const mrt::Serializator &s) {
TRY {
	s.get(_last_id);
	_last_id += 10000;
	
	unsigned int size;
	s.get(size);
	
	std::set<int> recv_ids;
	
	while(size--) {
		recv_ids.insert(deserializeObject(s)->_id);
	}
	cropObjects(recv_ids);	
} CATCH("World::deserialize()", throw;);
	//LOG_DEBUG(("deserialization completed successfully"));
}

void IWorld::generateUpdate(mrt::Serializator &s, const bool clean_sync_flag) {
	unsigned int c = 0, n = _objects.size();
	std::set<int> skipped_objects;

	for(ObjectMap::reverse_iterator i = _id2obj.rbegin(); i != _id2obj.rend(); ++i) {
		const Object *o = i->second;
		if (o->need_sync || o->speed != 0) {
			++c;
		} else skipped_objects.insert(o->_id);
	}
	LOG_DEBUG(("generating update %u objects of %u", c, n));

	s.add(c);
	for(ObjectMap::reverse_iterator i = _id2obj.rbegin(); i != _id2obj.rend(); ++i) {
		const int id = i->first;
		Object *o = i->second;
		if (skipped_objects.find(id) != skipped_objects.end()) 
			continue;
		
		serializeObject(s, o);	
		if (clean_sync_flag && o->need_sync)
			o->need_sync = false;
	}
	
	unsigned int skipped = skipped_objects.size();
	s.add(skipped);
	for(std::set<int>::const_iterator i = skipped_objects.begin(); i != skipped_objects.end(); ++i) {
		s.add(*i);
	}
	s.add(_last_id);
}

void IWorld::applyUpdate(const mrt::Serializator &s, const float dt) {
TRY {
	unsigned int n;
	std::set<int> skipped_objects;
	ObjectSet objects;
	s.get(n);
	while(n--) {
		Object *o = deserializeObject(s);
		if (o == NULL) {
			LOG_WARN(("some object failed to deserialize. wait for the next update"));
			continue;
		}
		objects.insert(o);
		skipped_objects.insert(o->_id);
	}
	s.get(n);
	while(n--) {
		int id;
		s.get(id);
		skipped_objects.insert(id);
	}
	s.get(_last_id);
	_last_id += 10000;
	TRY {
		cropObjects(skipped_objects);
	} CATCH("applyUpdate::cropObjects", throw;);
	TRY {
		tick(objects, dt);
	} CATCH("applyUpdate::tick", throw;);
} CATCH("applyUpdate", throw;)
}

void IWorld::serializeObjectInfo(mrt::Serializator &s, const int id) const {
	const Object * o = getObjectByID(id);
	if (o == NULL) 
		throw_ex(("serializeObjectInfo: no object %d", id));
	v3<float> pos, vel;
	o->getInfo(pos, vel);
	pos.serialize(s);
	vel.serialize(s);
}

Object * IWorld::deserializeObjectInfo(const mrt::Serializator &s, const int id, const bool fake) {
	Object * o = getObjectByID(id);
	if (o == NULL || fake) {
		v3<float> p;
		p.deserialize(s);
		p.deserialize(s);
		return o;
	}
	v3<float> pos, vel;
	o->_position.deserialize(s);
	o->_velocity.deserialize(s);
	return o;
}

const bool IWorld::getNearest(const Object *obj, const std::string &classname, v3<float> &position, v3<float> &velocity, Way * way) const {
	position.clear();
	velocity.clear();
	float distance = std::numeric_limits<float>::infinity();
	const Object *target = NULL;
	
	for(ObjectSet::const_iterator i = _objects.begin(); i != _objects.end(); ++i) {
		const Object *o = *i;
		//LOG_DEBUG(("%s is looking for %s. found: %s", obj->classname.c_str(), classname.c_str(), o->classname.c_str()));
		if (o->_id == obj->_id || o->classname != classname || 
			o->_owner_id == obj->_id || obj->_owner_id == o->_id || 
			(o->_owner_id != 0 && o->_owner_id == obj->_owner_id)
		) continue;

		v3<float> cpos = o->_position + o->size / 2;
		float d = obj->_position.quick_distance(cpos);
		if (d < distance) {
			distance = d;
			position = cpos;
			velocity = o->_velocity;
			target = o;
		}
	}
	if (target == NULL) 
		return false;
	
	position -= obj->_position + obj->size / 2;
	if (way == NULL)
		return true;
	return findPath(obj, position, *way, target);
}

//BIG PATHFINDING PART

typedef v3<int> vertex;
typedef std::deque<vertex> vertex_queue;

static inline void push(Matrix<int> &path, vertex_queue &buf, const vertex &vertex) {
	int w = path.get(vertex.y, vertex.x);
	if (w != -1 && w <= vertex.z) 
		return;
	path.set(vertex.y, vertex.x, vertex.z);
	buf.push_back(vertex);
}

static inline const bool pop(vertex_queue &buf, vertex &vertex) {
	if (buf.empty())
		return false;
	vertex = buf.front();
	buf.pop_front();
	return true;
}
/*
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
*/
	
const bool IWorld::findPath(const Object *obj, const v3<float>& position, Way & way, const Object *dst_obj) const {
	//finding shortest path.
	v3<float> tposition = obj->_position + position;
	
	Matrix<int> imp, path;
	World->getImpassabilityMatrix(imp, obj, dst_obj);
	//LOG_DEBUG(("imp\n%s", imp.dump().c_str()));
	
	v3<int> src = obj->_position.convert<int>() / IMap::pathfinding_step;
	v3<int> dst = tposition.convert<int>() / IMap::pathfinding_step;
	
	int w = imp.getWidth(), h = imp.getHeight();

	path.setSize(h, w, -1);
	path.useDefault(-1);
	
	vertex_queue buf;
	imp.set(src.y, src.x, 0);
	push(path, buf, vertex(src.x, src.y, 0));
	
	vertex v;
	while(pop(buf, v)) {
		int n = path.get(v.y, v.x);
		assert(n != -1);
		
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
/*
		//disabled diagonals for now
		if (check(imp, v, 1, 1) != -1)
			push(path, buf, vertex(v.x + 1, v.y + 1, n));
		if (check(imp, v, 1, -1) != -1)
			push(path, buf, vertex(v.x + 1, v.y - 1, n));
		if (check(imp, v, -1, 1) != -1)
			push(path, buf, vertex(v.x - 1, v.y + 1, n));
		if (check(imp, v, -1, -1) != -1)
			push(path, buf, vertex(v.x - 1, v.y - 1, n));
*/
	}
	
	int len, n = path.get(dst.y, dst.x);
	len = n;
	
	if (n == -1) {
		/*
		imp.set(dst.y, dst.x, -99);
		imp.set(src.y, src.x, imp.get(src.y, src.x) - 100);
		*/
		LOG_DEBUG(("path not found from %d:%d -> %d:%d", src.y, src.x, dst.y, dst.x));
		//LOG_DEBUG(("imp\n%s", imp.dump().c_str()));
		//LOG_DEBUG(("path\n%s", path.dump().c_str()));
		return false;
	}

	way.clear();
	int x = dst.x, y = dst.y;
	int vi = -10;
	
	while ( x != src.x || y != src.y) {
		assert(imp.get(y, x) != -1);
		imp.set(y, x, vi--);
		way.push_front(WayPoint(x, y, 0));
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
		/*
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
		*/
		assert(t != -1);
		
		x = x2; y = y2; n = t;
	}
	//way.push_front(WayPoint(x, y, 0));
	//LOG_DEBUG(("imp\n%s", imp.dump().c_str()));
	
	
	for(Way::iterator i = way.begin(); i != way.end(); ++i) {
		(*i) *= IMap::pathfinding_step;
	}
	
	//LOG_DEBUG(("getPath: length: %d, \n%s", len, result.dump().c_str()));
	return true;
}

