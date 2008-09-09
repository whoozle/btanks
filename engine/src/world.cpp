/* Battle Tanks Game
 * Copyright (C) 2006-2008 Battle Tanks team
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

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
*/

#include "world.h"
#include "animation_model.h"
#include "object.h"
#include "tmx/map.h"
#include "resource_manager.h"
#include "player_manager.h"
#include "player_slot.h"
#include "controls/control_method.h"
#include "config.h"
#include "utils.h"
#include "math/unary.h"
#include "rt_config.h"

#include "mrt/exception.h"
#include "mrt/logger.h"
#include "mrt/chunk.h"
#include "mrt/random.h"

#include "sdlx/rect.h"
#include "sdlx/surface.h"

#include "vehicle_traits.h"
#include "ai/traits.h"
#include "zbox.h"

#include <math.h>
#include <assert.h>
#include <limits>
#include "special_owners.h"
#include "math/binary.h"
#include "profiler.h"

#include "rotating_object.h"

static inline void call_calculate(Object *object, float dt) {
	RotatingObject *ro = dynamic_cast<RotatingObject *>(object);
	if (ro == NULL) {
		object->Object::calculate(dt);
	} else {
		ro->RotatingObject::calculate(dt);
	}
}


IMPLEMENT_SINGLETON(World, IWorld);

static Profiler profiler;

void IWorld::setTimeSlice(const float ts) {
	if (ts <= 0)
		throw_ex(("invalid timeslice value passed (%g)", ts));
	_max_dt = ts;
	LOG_DEBUG(("setting maximum timeslice to %g", _max_dt));
}

void IWorld::initMap() {
	if (_hp_bar == NULL)
		_hp_bar = ResourceManager->loadSurface("hud/hp.png");

	GET_CONFIG_VALUE("engine.grid-fragment-size", int, gfs, 128);
	_grid.set_size(Map->get_size(), gfs, Map->torus());
}

void IWorld::clear() {
	LOG_DEBUG(("cleaning up world..."));
	std::for_each(_objects.begin(), _objects.end(), delete_ptr2<ObjectMap::value_type>());
	_objects.clear();
	_grid.clear();
	
	_collision_map.clear();
	_static_collision_map.clear();
	
	_last_id = 0;
	_atatat = false;
	profiler.dump();
	_out_of_sync = -1;
	_out_of_sync_sent = -1;
	_current_update_id = -1;
}

void IWorld::setMode(const std::string &mode, const bool value) {
	if (mode == "atatat")  {
		_atatat = value;
	} else 
		throw_ex(("invalid mode '%s'", mode.c_str()));
}


IWorld::IWorld() : _last_id(0), _atatat(false), 
	_max_dt(1), _out_of_sync(-1), _out_of_sync_sent(-1), _current_update_id(-1), _hp_bar(NULL) {
	
	LOG_DEBUG(("world ctor"));
	init_map_slot.assign(this, &IWorld::initMap, Map->load_map_signal);
	map_resize_slot.assign(this, &IWorld::onMapResize, Map->map_resize_signal);
}

IWorld::~IWorld() {
	clear();
}

void IWorld::deleteObject(Object *o) {
	on_object_delete.emit(o);
	const int id = o->_id;
	for(StaticCollisionMap::iterator i = _static_collision_map.begin(); i != _static_collision_map.end(); ) {
		if (i->first.first == id || i->first.second == id) {
			_static_collision_map.erase(i++);
		} else {
			++i;
		}
	}
	_grid.remove(o);
	delete o;
}


void IWorld::updateObject(Object *o) {
	if (o->size.is0())
		return;
	
	Map->validate(o->_position);
	_grid.update(o, o->_position.convert<int>(), o->size.convert<int>());
	on_object_update.emit(o);
}

void IWorld::addObject(Object *o, const v2<float> &pos, const int id) {
	if (o == NULL) 
		throw_ex(("adding NULL as world object is not allowed"));
	o->_id = (id > 0)?id:++_last_id;
	
	ObjectMap::iterator existing_object = _objects.find(o->_id);
	if (PlayerManager->is_client() && existing_object != _objects.end()) {
		//client mode: actual id may overlap due possible races.
		//note that it's temporary solution: on the next cropping update
		//this objects will be killed or replaced :)
		if (id > 0) {
			Object *eo = existing_object->second;
			_grid.remove(eo);
			delete eo;
			existing_object->second = o;
		} else {
			ObjectMap::iterator i;
			for(i = existing_object; i != _objects.end(); ++i) {
				Object *eo = i->second;
				if (eo->_dead) {
					_grid.remove(eo);
					delete eo; 

					o->_id = i->first;
					i->second = o;
					break;
				}
			}

			if (i == _objects.end()) {
				o->_id = _objects.rbegin()->first + 1;
				assert(_objects.find(o->_id) == _objects.end());
				_objects.insert(ObjectMap::value_type(o->_id, o));
			}
		}
		
	} else {
		assert(o->_id > 0);
		assert (existing_object == _objects.end());
		_objects.insert(ObjectMap::value_type(o->_id, o));
	}

	o->_position = pos;
	
	if (o->_variants.has("ally")) {
		o->remove_owner(OWNER_MAP);
		o->prepend_owner(OWNER_COOPERATIVE);
	}

	assert(o->_group.empty());
	o->on_spawn();

	on_object_add.emit(o);
	updateObject(o);

	GET_CONFIG_VALUE("engine.enable-profiler", bool, ep, false);
	if (ep) {
		profiler.create(o->registered_name);
	}

	o->invalidate();
	//LOG_DEBUG(("object %d added, objects: %d", o->_id, _objects.size()));
}

#include "game_monitor.h"
#include "mrt/timespy.h"

struct ObjectZCompare {
	inline bool operator()(const Object * a, const Object * b) const {
		return a->get_z() != b->get_z()? a->get_z() > b->get_z(): a > b; //hack to maintain object order with equal range
	}
};

void IWorld::render(sdlx::Surface &surface, const sdlx::Rect& src, const sdlx::Rect &dst, const int _z1, const int _z2, const Object * player) {
	bool fog = false;
	
	if (player != NULL)
		Config->get("engine.fog-of-war.enabled", fog, false);
	
	v2<int> player_pos;
	sdlx::Rect fog_rect;
	if (fog) {
		GET_CONFIG_VALUE("engine.fog-of-war.width", int, fog_w, 800);
		GET_CONFIG_VALUE("engine.fog-of-war.height", int, fog_h, 600);
		player->get_center_position(player_pos);
		
		fog_rect.x = player_pos.x - dst.w / 2;
		fog_rect.y = player_pos.y - dst.h / 2;
		
		if (dst.w > fog_w) {
			fog_rect.x = player_pos.x - fog_w / 2;
			fog_rect.w = fog_w;
			fog_rect.h = fog_h;
		}
		if (dst.h > fog_h) {
			fog_rect.y = player_pos.y - fog_h / 2;
			fog_rect.w = fog_w;
			fog_rect.h = fog_h;
		}
		if (fog_rect.w == 0)
			fog = false;
		//LOG_DEBUG(("fog: %s", fog?"true":"false"));
	}
	
	
	GET_CONFIG_VALUE("engine.render-hp-bars", bool, rhb, false);
	std::vector<v3<int> > specials; 
	specials = GameMonitor->getSpecials();

	std::set<int> special_ids;
	for(size_t i = 0; i < specials.size(); ++i) {
		special_ids.insert(specials[i].z);
	}
	
	surface.set_clip_rect(dst);
	typedef std::priority_queue<Object *, std::deque<Object *>, ObjectZCompare> LayerMap;
	LayerMap layers;
	const IMap &map = *Map.get_const();
	GET_CONFIG_VALUE("engine.show-waypoints", bool, show_waypoints, false);

	std::set<Object *> objects; 
	_grid.collide(objects, v2<int>(src.x, src.y), v2<int>(dst.w, dst.h));
	//LOG_DEBUG(("render: collide returns %u objects", (unsigned)objects.size()));
	for(std::set<Object *>::iterator i = objects.begin(); i != objects.end(); ++i) {
		Object *o = *i;
		assert(o != NULL);
		if (o->is_dead() || o->skip_rendering()) {
			//LOG_DEBUG(("render: skipped dead object: %s", o->registered_name.c_str()));
			continue;
		}
		
		std::set<Object *> objects;
		objects.insert(o);
		o->get_subobjects(objects);
		for(std::set<Object *>::iterator j = objects.begin(); j != objects.end(); ++j) {
			Object *o = *j;
			assert(o != NULL);
		
			if (o->_z < _z1 || o->_z >= _z2) 	
				continue;
			
			v2<float> position;
			o->get_position(position);
			sdlx::Rect r((int)position.x, (int)position.y, (int)o->size.x, (int)o->size.y);
			bool fogged = fog;// && o->speed != 0;
			//LOG_DEBUG(("%d,%d:%d,%d vs %d,%d:%d,%d result: %s", 
			//	r.x, r.y, r.w, r.h, src_rect.x, src_rect.y, src_rect.w, src_rect.h, Map->intersects(r, src_rect)?"true":"false"));
			if (Map->intersects(r, fogged? fog_rect: src) || (show_waypoints && o->is_driven())) 
				layers.push(o);
		}
	}

	//LOG_DEBUG(("rendering %u objects", (unsigned)layers.size()));
	v2<int> map_size = Map->get_size(), map_tile_size = map_size / Map->getTileSize();
	int z1 = _z1;
	while(!layers.empty()) {
		Object *o = layers.top();
		layers.pop();
		
		assert(o != NULL);
		if (o->is_dead())
			continue;
		
		int z2 = o->get_z();
		if (z2 < z1) 
			continue;

		if (z2 >= _z2)
			break;
		//LOG_DEBUG(("world::render(%d, %d)", z1, z2));
		if (z1 != z2) {
			//LOG_DEBUG(("calling map::render(%d, %d)", z1, z2));
			map.render(surface, src, dst, z1, z2);
		}
		z1 = z2;
		//LOG_DEBUG(("rendering %s with %d,%d", o.animation.c_str(), (int)o._position.x - src.x + dst.x, (int)o._position.y - src.y + dst.y));
		v2<float> position;
		o->get_position(position);
		v2<int> screen_pos((int)position.x - src.x, (int)position.y - src.y);
		if (Map->torus()) {
			screen_pos %= map_size;
			if (screen_pos.x < 0 && screen_pos.x + o->size.x < 0)
				screen_pos.x += map_size.x;
			if (screen_pos.y < 0 && screen_pos.y + o->size.y < 0)
				screen_pos.y += map_size.y;
		}
		//Map->validate(screen_pos);
		//LOG_DEBUG(("object: %s(%gx%g), position: %d %d", o.animation.c_str(), o.size.x, o.size.y, screen_pos.x, screen_pos.y));

		o->render(surface, screen_pos.x + dst.x, screen_pos.y + dst.y);
		
		const Way & way = o->get_way();
		if (show_waypoints && !way.empty()) {
			const Animation *a = ResourceManager.get_const()->getAnimation("waypoint-16");
			assert(a != NULL);
		
			const sdlx::Surface * wp_surface = ResourceManager->loadSurface(a->surface);

			for(Way::const_iterator wi = way.begin(); wi != way.end(); ++wi) {
				const v2<int> &wp = *wi;
				surface.blit(*wp_surface, 
					wp.x - src.x + dst.x - 8, wp.y - src.y + dst.y - 8);
			}
		}
		if (o->hp >= 20 && o->_parent == NULL && (special_ids.find(o->get_id()) != special_ids.end() || (rhb && (o->impassability == 1.0f && !o->piercing)))) {
			int h = _hp_bar->get_height() / 16;
			int y = (o->hp >= 0)?15 * (o->max_hp - o->hp) / o->max_hp: 0;
			sdlx::Rect hp_src(0, y * h, _hp_bar->get_width(), h);
			surface.blit(*_hp_bar, hp_src, 
					(int)o->_position.x - src.x + dst.x + (int)(o->size.x) - _hp_bar->get_width() - 4, 
					(int)o->_position.y - src.y + dst.y + 4);
		}
	}
	map.render(surface, src, dst, z1, _z2);
	if (show_waypoints) 
		GameMonitor->renderWaypoints(surface, src, dst);
	if (fog) {
		static const sdlx::Surface * fog_surface = ResourceManager->loadSurface("fog_of_war.png");
		int tw = fog_surface->get_width() / 3, th = fog_surface->get_height() / 3;
		
		int fog_tw = (fog_rect.w - 1) / tw + 1, fog_th = (fog_rect.h - 1) / th + 1;
		//LOG_DEBUG(("fog_rect: %d %d %d %d @%d,%d", fog_rect.x, fog_rect.y, fog_rect.w, fog_rect.h, fog_tw, fog_th));
		
		int dst_tw = (dst.w - 1) / tw + 1, dst_th = (dst.h - 1) / th + 1;
		
		fog_tw |= 1; fog_th |= 1;
		dst_tw |= 1; dst_th |= 1;
		if (fog_tw <= 1 || fog_th <= 1)
			throw_ex(("fog player window is far too small for that"));

		sdlx::Rect fog_src(tw, th, tw, th);
		
		int px = (player_pos.x - src.x) / tw, py = (player_pos.y - src.y) / th;
		if (Map->torus()) {
			px %= map_tile_size.x;
			py %= map_tile_size.x;
			if (px < 0)
				px += map_tile_size.x;
			if (py < 0)
				py += map_tile_size.y;
		}
			
		//LOG_DEBUG(("player: %d %d, fog_tile_size: %d %d", px, py, fog_tw, fog_th));
		int dx = (player_pos.x - src.x) % tw, dy = (player_pos.y - src.y) % th;
		int cx = fog_tw / 2, cy = fog_th / 2;
		
		for(int y = -1; y <= dst_th; ++y) {
			for(int x = -1; x <= dst_tw; ++x) {
				int rx = math::abs(x - px), ry = math::abs(y - py);
				if (rx < cx && ry < cy)
					continue;

				fog_src.x = tw;
				fog_src.y = th;
				if (rx == cx && ry <= cy) {
					fog_src.x = (x < px)?0:2 * tw;
				}
				
				if (ry == cy && rx <= cx) {
					fog_src.y = (y < py)?0:2 * th;
				}
				surface.blit(*fog_surface, fog_src, dst.x + x * tw + dx - tw / 2, dst.y + y * th + dy - th / 2);
			}
		}
	}	
	surface.reset_clip_rect();
}

const bool IWorld::collides(Object *obj1, const v2<int> &position, Object *obj2, const bool probe) const {
	TRY {
		const int id1 = obj1->_id;
		const int id2 = obj2->_id;
		assert(obj1 != NULL && obj2 != NULL);

		if (id1 == id2 || 
			(obj1->impassability < 1.0 && obj1->impassability >= 0) || 
			(obj2->impassability < 1.0 && obj2->impassability >= 0) || 
			(obj1->piercing && obj2->pierceable) || (obj1->pierceable && obj2->piercing) ||
			obj1->is_dead() || obj2->is_dead() ||
			//owner stuff
			obj1->has_same_owner(obj2, true) 
		) {
			return false;
		}

		
		CollisionMap::key_type key = (id1 < id2) ? CollisionMap::key_type(id1, id2): CollisionMap::key_type(id2, id1);
		
		if (!probe) {
			CollisionMap::const_iterator i = _collision_map.find(key);
		 	if (i != _collision_map.end()) {
			 	return i->second;
			}
		 }
		//LOG_DEBUG(("collides(%s:%d(speed: %g), (%d, %d), %s:%d(speed: %g), %s)", obj->registered_name.c_str(), obj->_id, obj->speed, position.x, position.y, o->registered_name.c_str(), o->_id, o->speed, probe?"true":"false"));
		
		v2<int> dpos = obj2->_position.convert<int>() - position;
		//LOG_DEBUG(("%s: %d %d", o->classname.c_str(), dpos.x, dpos.y));
		
		bool collides;
		if (obj1->speed == 0 && obj2->speed == 0) {
			//static objects.
			StaticCollisionMap::iterator static_i = _static_collision_map.find(key);
			int p1 = id1 < id2 ? (int)obj1->_pos : (int)obj2->_pos;
			int p2 = id1 < id2 ? (int)obj2->_pos : (int)obj1->_pos;
			if (static_i != _static_collision_map.end() && p1 == static_i->second.first && p2 == static_i->second.second) {
				collides = static_i->second.third;
			} else {
				collides = obj1->collides(obj2, dpos.x, dpos.y);
				_collision_map.insert(CollisionMap::value_type(key, collides));
				 ternary<int, int, bool> value = id1 < id2 ? ternary<int, int, bool>((int) obj1->_pos, (int) obj2->_pos, collides): ternary<int, int, bool>((int) obj2->_pos, (int) obj1->_pos, collides);
				_static_collision_map.insert(StaticCollisionMap::value_type(key, value)); 
			}
		} else {
			collides = obj1->collides(obj2, dpos.x, dpos.y);
		}

		//LOG_DEBUG(("collision %s <-> %s: %s", obj->classname.c_str(), o->classname.c_str(), collides?"true":"false"));
		if (!probe) {
			_collision_map.insert(CollisionMap::value_type(key, collides));
		
			if (collides) { 
				//LOG_DEBUG(("collision %s <-> %s", obj->classname.c_str(), o->classname.c_str()));
			
				/*
				float m = obj->mass / o->mass;
				if (m > 1.0) 
				m = 1.0;
				v2<float> o_vf = o->_velocity * -m, obj_vf = obj->_velocity * (-1/m);
				*/
				obj2->emit("collision", obj1);
				obj1->emit("collision", obj2);
			
				if (obj1->is_dead() || obj2->is_dead() || obj1->impassability == 0 || obj2->impassability == 0) {
					return false; //the most common case is the bullet which collides with object.
				}
			
			}
		}
		//LOG_DEBUG(("collision %s <-> %s: %s", obj->classname.c_str(), o->classname.c_str(), collides?"true":"false"));
		
		return collides;
	} CATCH(
		mrt::format_string("World::collides(%p, (%d:%d), %p, %s)", (void *)obj1, position.x, position.y, (void *)obj2, probe?"true":"false").c_str(), 
		throw; )
	return 0;
}

const float IWorld::getImpassability(Object *obj, const v2<int> &position, const Object **collided_with, const bool probe, const bool skip_moving) const {
TRY {
	assert(obj != NULL);
	
	if (obj->impassability == 0) {
		if (collided_with != NULL)
			*collided_with = NULL;
		return 0;
	}

/*	LOG_DEBUG(("%d:%s->getImpassability((%d, %d)%s%s)", 
		obj->get_id(), obj->animation.c_str(), 
		position.x, position.y, 
		(probe?" [probe]":""), (skip_moving?" [skip_moving]":"")));
*/

	float im = 0;
	const Object *result = NULL;
	
	sdlx::Rect my((int)position.x, (int)position.y,(int)obj->size.x, (int)obj->size.y);


	std::set<Object *> objects;
	_grid.collide(objects, position, obj->size.convert<int>());
	//consult grid

	for(std::set<Object *>::const_iterator i = objects.begin(); i != objects.end(); ++i) {
		Object *o = *i;
		
		if (obj->speed == 0 && o->impassability < 1 && o->impassability >= 0)
			continue;
		if (obj->_id == o->_id || o->impassability == 0 || (skip_moving && o->speed != 0))
			continue;
		if (!ZBox::sameBox(obj->_z, o->_z))
			continue;

		sdlx::Rect other((int)o->_position.x, (int)o->_position.y,(int)o->size.x, (int)o->size.y);
		if (!Map->intersects(my, other)) 
			continue;

		if (!collides(obj, position, o, probe)) 
			continue;
		
		if (o->impassability > im) {
			im = o->impassability;
			result = o;
			if (im >= 1.0)
				break;
		}

	}
	if (collided_with != NULL)
		*collided_with = result;
	
	return obj->get_effective_impassability(im);
} CATCH(mrt::format_string("World::getImpassability(%p, (%d, %d), %p, %s, %s)", 
	(void *)obj, position.x, position.y, (void *)collided_with, probe?"true":"false", skip_moving?"true":"false").c_str(), 
	throw;);	
	return 0;
}

void IWorld::get_impassability_matrix(Matrix<int> &matrix, const Object *src, const Object *dst) const {
	const v2<int> size = Map->getTileSize();
	const v2<int> tile_size = Map->getTileSize();
	int z = 0;
	if (src)
		z = src->get_z();

	GET_CONFIG_VALUE("map.pathfinding-step", int, ps, 32);
	const int split = 2 * ((tile_size.x - 1) / 2 + 1) / ps;

	matrix = Map->get_impassability_matrix(z);
	for(ObjectMap::const_iterator i = _objects.begin(); i != _objects.end(); ++i) {
		Object *o = i->second;
		if (o == src || o == dst || o->impassability <= 0 || o->piercing)
			continue;
		if (src != NULL && !ZBox::sameBox(src->_z, o->_z))
			continue;
		
		int im = (int)(o->impassability * 100);
		if (im >= 100)
			im = -1;
		
/*
		v2<int> p1, p2;
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

		v2<int> p = ((o->_position + o->size/2) / tile_size.convert<float>()).convert<int>();
		Matrix<bool> proj;
		o->check_surface();
		o->_cmap->project(proj, split, split);
		//LOG_DEBUG(("projection: %s", proj.dump().c_str()));
		//_imp_map.set(y, x, im);
		for(int yy = 0; yy < split; ++yy)
			for(int xx = 0; xx < split; ++xx) {
				int yp = p.y * split + yy, xp = p.x * split + xx;
				if (proj.get(yy, xx) && matrix.get(yp, xp) >= 0) 
					matrix.set(yp, xp, im);
			}
	}
	//LOG_DEBUG(("projected objects:\n%s", matrix.dump().c_str()));
}

#include "ai/synchronizable.h"

void IWorld::_tick(Object &o, const float dt, const bool do_calculate) {
	if (o.is_dead()) 
		return;

	//LOG_DEBUG(("tick object %p: %d: %s", (void *)&o, o.get_id(), o.animation.c_str()));
	GET_CONFIG_VALUE("engine.speed", float, e_speed, 1.0f);

	const IMap &map = *IMap::get_instance();
	v2<int> map_size = map.get_size();

TRY {

	if (o.ttl > 0) {
		o.ttl -= dt * e_speed;
		if (o.ttl <= 0) {
			//dead
			o.emit("death");
			o.ttl = 0;
		}
	}
	if (o.is_dead()) 
		return;

} CATCH("ttl decrementing", throw;);		


	v2<float> old_vel = o._velocity;
	bool drifting = false;

	if (_atatat && !o.piercing && o.mass > 20) {
		if (!o.has("atatat-tooltip")) {
			o.add("atatat-tooltip", "random-tooltip", "skotobaza", v2<float>(48, -48), Centered);
		}

		PlayerState state = o.get_player_state();
		state.fire = true;
		state.alt_fire = true;
		
		static Alarm update_state(2.0, true);
		if (!o.disable_ai && update_state.tick(dt)) {
			update_state.reset();
			int n = mrt::random(2);
			int k1 = mrt::random(2), k2 = mrt::random(2);
			if (n == 0) {
				//one key
				if (k1 == 0) {
					state.left  = k2 == 0;
					state.right = k2 != 0;
				} else {			
					state.up   = k2 == 0;
					state.down = k2 != 0;
				}	
			} else {
				state.left  = k1 == 0;
				state.right = k1 != 0;
				state.up   = k2 == 0;
				state.down = k2 != 0;
			}
		}
		
		o.update_player_state(state);
		call_calculate(&o, dt);
	} else if (o.has_effect("drifting")) {
		//drifting
		drifting = true;
		//LOG_DEBUG(("drifting! %s", o.animation.c_str()));
		GET_CONFIG_VALUE("engine.drifting-duration", float, dd, 0.1);
	//	GET_CONFIG_VALUE("engine.drifting-rotation", float, dr, 0.03);

		float t = o.get_effect_timer("drifting");
		//LOG_DEBUG(("%g", t));
		if (t == dd) {
			TRY { 
				if (o.disable_ai) {
					call_calculate(&o, dt);
				} else {
					o.calculate(dt);
				}
			} CATCH("calling o.calculate", throw;)
		}
		if (o._velocity.is0() && o.get_directions_number() > 1) 
			o._velocity.fromDirection(o._direction_idx, o.get_directions_number());
				
	} else if (do_calculate) {
		//regular calculate
		GET_CONFIG_VALUE("engine.enable-profiler", bool, ep, false);
		TRY { 
			PlayerState old_state = o.get_player_state();
			if (o.has_effect("obey")) {
				//LOG_DEBUG(("obey!!!"));
				if (o.is_driven()) {
					o.calculate_way_velocity();
				} else {
					o._velocity.clear();
				}
				o.limit_rotation(dt, 0.1f, true, false);
			} else if (o.disable_ai) {
				call_calculate(&o, dt);
			} else {
				if (ep) {
					profiler.reset();
					o.calculate(dt);
					profiler.add(o.registered_name, dt);
				} else {
					o.calculate(dt);
				}
			}
			if (old_state != o.get_player_state() && dynamic_cast<ai::Synchronizable *>(&o) != NULL) {
				//LOG_DEBUG(("buratino %s changed state", o.animation.c_str()));
				PlayerManager->send_object_state(o.get_id(), o.get_player_state());
			}
		} CATCH("calling o.calculate", throw;)
	}

TRY {
	if(o.get_player_state().leave) {
		//if (!detachVehicle(&o))
		//	o.get_player_state().leave = false; //do not trigger MP stuff. :)
		o.detachVehicle();
	}
} CATCH("detaching from vehicle", throw;)
	if (o._dead)
		return;

	//interpolation stuff
	if (o._interpolation_progress < 1.0) {
		GET_CONFIG_VALUE("multiplayer.interpolation-duration", float, mid, 0.2);	
		if (mid <= 0)
			throw_ex(("multiplayer.interpolation-duration must be greater than zero"));
		
		float dp = dt / mid, dp_max = 1.0 - o._interpolation_progress;
		o._interpolation_progress += dp;
		
		if (dp > dp_max)
			dp = dp_max;
		
		Map->add(o._position, o._interpolation_vector * dp);
	} 
		
	TRY { 
		bool tap = o.has_effect("tap");
		bool cork = o.has_effect("cork");
		PlayerState state_backup = o.get_player_state();
		if (tap) 
			o._state.fire = false;
		if (cork)
			o._state.alt_fire = false;
			
		o.tick(dt);

		if (state_backup.fire && tap)
			o._state.fire = state_backup.fire;
		if (state_backup.alt_fire && cork)
			o._state.alt_fire = state_backup.alt_fire;
	} CATCH("calling o.tick", throw;)

	if (o._dead)
		return;

	TRY {
		o.group_tick(dt);
	} CATCH("calling group_tick", throw);

	const std::string outline_animation = o.animation + "-outline";
	const bool has_outline = ResourceManager->hasAnimation(outline_animation);
	bool hidden = false;
	//LOG_DEBUG(("outline: %s", outline_animation.c_str()));

	if (o.speed == 0 && o.impassability != -1) {
		TRY {
			v2<int> pos = o._position.convert<int>();
			if (o.impassability < 0 || o.impassability >= 1.0f) {
				if (has_outline) {
					map.getImpassability(&o, pos, NULL, has_outline? &hidden: NULL);
					//LOG_DEBUG(("o: %s, hidden: %s", o.animation.c_str(), hidden?"yes":"no"));
					o.update_outline(hidden);
				}
				//getImpassability(&o, pos);
			}
		} CATCH("tick(speed==0)", throw;);

		return;
	}
		
	float len = o._velocity.normalize();
		
	if (len == 0) {
		if (o.impassability < 0 || o.impassability >= 1.0f) {
			getImpassability(&o, o._position.convert<int>());
		}
		return;
	} 
	
	if (PlayerManager->is_server_active())
		o.delta_distance_stat += (int)len;
	
	/*
	GET_CONFIG_VALUE("engine.mass-acceleration-divisor", float, ac_div, 1000.0);

	const float ac_t = o.mass / ac_div;
	if (o.mass > 0 && o._moving_time < ac_t) {
		o._velocity *= o._moving_time / ac_t;
	}
	*/

	v2<int> old_pos = o._position.convert<int>();

	const Object *stuck_in = NULL;
	IMap::TilePosition stuck_map_pos;

	float map_im_now = o.piercing?0:(map.getImpassability(&o, old_pos, &stuck_map_pos) / 100.0f);
	float obj_im_now = o.piercing?0:getImpassability(&o, old_pos, &stuck_in, true);
	float result_im = math::max(map_im_now, obj_im_now);

	float obj_speed = o.speed;
	
	{
		const bool speedup = o.has_effect("speedup");
		const bool slowdown = o.has_effect("slowdown");
		if (speedup && slowdown) {
			o.remove_effect("speedup");
			o.remove_effect("slowdown");
		} else if (speedup) {
			GET_CONFIG_VALUE("objects.speedup-item.speedup-factor", float, su, 1.5f);
			obj_speed *= su;
		} else if (slowdown) {
			GET_CONFIG_VALUE("objects.slowdown-item.slowdown-factor", float, su, 1.5f);
			obj_speed /= su;
		}
	}
	
	v2<float> dpos = e_speed * obj_speed * o._velocity * dt * (1.0f - result_im);

	bool stuck = result_im >= 1.0f;

	v2<int> new_pos = (o._position + dpos).convert<int>();
	Map->validate(new_pos);

/*
	DOUBLE CHECK IT
	if (!stuck && new_pos == old_pos) {
		o._position += dpos;
		return;
	}
*/
	
	const Object *other_obj = NULL;

	int attempt = -1;

	float map_im = 0, obj_im = 0;
	
	GET_CONFIG_VALUE("engine.debug-stuck-resolution-code", bool, dorc, false);
	if (dorc && !o._variants.has("player"))
		dorc = false;

TRY {	
	
	int save_dir = o.get_direction();
	int dirs = o.get_directions_number();
	bool hidden_attempt[5] = { false, false, false, false, false };
	
	v2<float> new_velocity;
	
	GET_CONFIG_VALUE("engine.disable-sliding", bool, ds, false);

	for(attempt =0; attempt < 5; ++attempt) {
		v2<int> pos;
		if (attempt == 0) {
			pos = new_pos;
			new_velocity = o._velocity;
		} else if (attempt >= 1 && attempt <= 2) { 
			int dir = save_dir;
			dir = (dir + ((attempt == 1)?-1:1) + dirs ) % dirs;
			
			new_velocity.fromDirection(dir, dirs);

			float im = (result_im < 1.0f)?result_im:0.9f;
			pos = (o._position + (1.0f - im) * e_speed * obj_speed * new_velocity * dt).convert<int>();
			o.set_direction(dir);
			//LOG_DEBUG(("%s: %d:trying %d (original: %d, dirs: %d)", 
			//	o.animation.c_str(), attempt, dir, save_dir, dirs ));
		} else if (attempt >= 3) {
			o.set_direction(save_dir);
			int dir = save_dir;
			dir = (dir + (attempt == 3?-dirs/4:dirs/4) + dirs ) % dirs;
			
			new_velocity.fromDirection(dir, dirs); //position
			new_velocity *= 7;
			
			if (dorc)
				LOG_DEBUG(("new position delta[strafe workaround]: %g, %g", new_velocity.x, new_velocity.y));

			float im = (result_im < 1.0f)?result_im:0.9f;
			pos = (new_velocity + o._position + (1.0f - im) * e_speed * obj_speed * o._velocity * dt).convert<int>();
		}
		Map->validate(pos);
		
		map_im = map.getImpassability(&o, pos, NULL, has_outline?(hidden_attempt + attempt):NULL) / 100.0f;
		obj_im = getImpassability(&o, pos, &other_obj, attempt > 0);  //make sure no cached collision event reported here
		GET_CONFIG_VALUE("engine.no-clip", bool, no_clip, false);
		if (no_clip) {
			LOG_DEBUG(("map im: %g, obj im: %g", map_im, obj_im));
			map_im = obj_im = 0;
		}
		if (o.impassability < 0) {
			map_im = 0;
			obj_im = 0;
			result_im = 0;
			stuck = false;
			break;
		} 
		if (map_im >= 0 && map_im < 1.0 && obj_im < 1.0) {
			//LOG_DEBUG(("success, %g %g", map_im, obj_im));
			if (result_im >= 1.0f) {
				result_im = math::max(map_im, obj_im);
				dpos = e_speed * obj_speed * (1.0f - result_im) * o._velocity * dt;
			}
			stuck = false;
			break;
		}
		
		if (o.piercing || dirs == 1)
			break;
	
		if (ds || (other_obj != NULL && o.disable_ai && other_obj->disable_ai))
			break;

		if (dorc)
		LOG_DEBUG(("(%d:%d->%d:%d): (attempt %d) stuck: %s, map_im: %g, obj_im: %g, obj_im_now: %g", 
				old_pos.x, old_pos.y, (int)pos.x, (int)pos.y, attempt,
				stuck?"true":"false", map_im, obj_im, obj_im_now));
		
	}
	
	if (attempt == 1 || attempt == 2) {
		o._velocity = new_velocity;
		hidden = hidden_attempt[attempt];
	} else if (attempt == 3 || attempt == 4) {
		Map->add(o._position, new_velocity);
		hidden = hidden_attempt[attempt];
		o.set_direction(save_dir);
	} else {
		o.set_direction(save_dir);
		hidden = hidden_attempt[0];
		if (attempt >= 3) 
			stuck = true;
	}

	result_im = (o.piercing)?0: math::max(map_im, obj_im);
	dpos = e_speed * obj_speed * (1.0f - result_im) * o._velocity * dt;

} CATCH(
	mrt::format_string("tick.impassability check (attempt: %d, stuck_in: %p)", attempt, (void *)stuck_in).c_str(), 
	throw;);

	o.update_outline(hidden);

TRY {
	if (o.piercing) {
		//if (obj_im_now > 0 && obj_im_now < 1.0)
		int dirs = o.get_directions_number();
		if (map_im >= 1.0f) {
			v2<float> dpos;
			if (dirs == 4 || dirs == 8 || dirs == 16) {
				dpos.fromDirection(o.get_direction(), o.get_directions_number());
				dpos *= o.size.length() / 2;
			}

			Map->damage(o._position + o.size / 2 + dpos, o.max_hp);
			o.emit("collision", NULL); //fixme: emit collisions with map from map::getImpassability
		} 
		map_im = 0;
		obj_im = 0; //collision handler was already called.
		stuck = false;
	}
} CATCH("tick(damaging map)", throw;)	

TRY {
	if (stuck && obj_speed != 0) {
			assert(!o.piercing);
			
			GET_CONFIG_VALUE("engine.stuck-resolution-steps", int, steps, 24);
			GET_CONFIG_VALUE("engine.stuck-resolution-step-size", int, step_size, 8);
			
			int a;

			//static const int directions[8] = {4, 3, 5, 0,  2, 6, 1, 7};
			static const int directions[8] = {0, 1, 7, 2, 6, 4, 5, 3};
			
			int dir = o.get_direction();
			int dirs = o.get_directions_number();
			if (dirs == 1) //this is temp hack to do not allow trains and other stupid objects to be corrected (and moved by player)
				goto skip_collision; 
			
			v2<int> pos;
			v2<float> dp;
			
			v2<int> size = Map->get_size();
			sdlx::Rect map_rect(0, 0, size.x, size.y);
			
			for(a = 0; a < steps; ++a) {
				for(int d = 0; d < 8; ++d) {
					dp.fromDirection((dir + directions[d] + 8) % 8, 8);
					dp *= (a + 1) * step_size;
					pos = (o.get_position() + dp).convert<int>();
					v2<int> c_pos = (o.get_center_position() + dp).convert<int>();
					if (!Map->torus() && !map_rect.in(c_pos.x, c_pos.y))
						continue;
					Map->validate(c_pos);

					float map_im = map.getImpassability(&o, pos, NULL, NULL) / 100.0f;
					float obj_im = getImpassability(&o, pos, NULL, true);
					if (obj_im < 1.0f && map_im < 1.0f) 
						goto found;
				}
			}
		found: 
			if (a >= steps) {
				LOG_DEBUG(("%d: %s couldnt escape from this cruel world. committing suicide. good luck in your next life...", 
					o._id, o.animation.c_str()));
				o.emit("death", NULL);
				goto skip_collision;
			}

			o._interpolation_vector = dp;
			o._interpolation_progress = 0;
			
		}
	skip_collision:;
	
} CATCH("tick(`stuck` case)", throw;);

	if (o.is_dead())
		return;

	result_im = math::max(map_im, obj_im);
	dpos = e_speed * obj_speed * o._velocity * dt * (1.0f - result_im);
	if (!dpos.is0()) {
		o._direction = o._velocity;
	}


//	if (o.piercing) {
//		LOG_DEBUG(("%s *** %g,%g, dpos: %g %g", o.dump().c_str(), map_im, obj_im, dpos.x, dpos.y));
//	}
	
TRY {
	assert(map_im >= 0 && obj_im >= 0);
	//LOG_DEBUG(("%s: %d %d: obj_im: %g, map_im: %g, dpos: %g %g %s", o.animation.c_str(), old_pos.x, old_pos.y, obj_im, map_im, dpos.x, dpos.y, stuck?"stuck":""));
	GET_CONFIG_VALUE("engine.drifting-impassability", float, dim, 0.4);
	if (drifting && obj_im < dim)
		obj_im = dim;
	
	new_pos = (o._position + dpos).convert<int>();
	
	if (!Map->torus()) {

		if (!o.piercing) {
			if ((dpos.x < 0 && new_pos.x < -o.size.x / 2) || (dpos.x > 0 && new_pos.x + o.size.x / 2 >= map_size.x))
				dpos.x = 0;

			if ((dpos.y < 0 && new_pos.y < -o.size.y / 2) || (dpos.y > 0 && new_pos.y + o.size.y / 2 >= map_size.y))
				dpos.y = 0;
		
		} else {
			if ((dpos.x < 0 && new_pos.x < -1.5 * o.size.x) || (dpos.x > 0 && new_pos.x >= map_size.x + 1.5 * o.size.x))
				dpos.x = 0;

			if ((dpos.y < 0 && new_pos.y < -1.5 * o.size.y) || (dpos.y > 0 && new_pos.y >= map_size.y + 1.5 * o.size.y))
				dpos.y = 0;
	
		}
	}
	Map->add(o._position, dpos);
	updateObject(&o);
	
} CATCH("tick(final)", throw;);
}


void IWorld::tick(const float dt) {
	//LOG_DEBUG(("tick dt = %f", dt));
	_collision_map.clear();
	tick(_objects, dt, true);
}

void IWorld::tick(Object &o, const float dt, const bool do_calculate) {
	if (dt < 0.001f && dt > -0.001f)
		return;

	_tick(o, dt, do_calculate);
}

void IWorld::tick(ObjectMap &objects, const float dt, const bool do_calculate) {
	if (dt < 0.001f && dt > -0.001f) {
		return;
	}

	float max_dt = dt >= 0? _max_dt: -_max_dt;
	int n = math::abs((int)(dt / max_dt));
	GET_CONFIG_VALUE("engine.trottle-slices", int, max_slices, 4);

	if (n > max_slices) {
		//LOG_DEBUG(("trottling needed (%d)", n));
		max_dt = dt / max_slices;
	}

	float dt2 = dt;
	if (dt > 0) {
		while(dt2 > max_dt) {
			_tick(objects, max_dt, do_calculate);
			dt2 -= max_dt;
		}
		if (dt2 > 0) 
			_tick(objects, dt2, do_calculate);
	} else if (dt < 0) {
		while(dt2 < max_dt) {
			_tick(objects, max_dt, do_calculate);
			dt2 -= max_dt;
		}
		if (dt2 < 0) 
			_tick(objects, dt2, do_calculate);
	}
}


void IWorld::_tick(ObjectMap &objects, const float dt, const bool do_calculate) {
	for(ObjectMap::iterator i = objects.begin(); i != objects.end(); ++i) {
		Object *o = i->second;
		assert(o != NULL);
		TRY {
			_tick(*o, dt, do_calculate);
		} CATCH(mrt::format_string("tick for object[%p] id:%d %s:%s:%s", (void *)o, o->get_id(), o->registered_name.c_str(), o->classname.c_str(), o->animation.c_str()).c_str(), throw;);
	}
}

void IWorld::purge(const float dt) {
	purge(_objects, dt);
}

void IWorld::purge(ObjectMap &objects, const float dt) {
	for(Commands::iterator i = _commands.begin(); i != _commands.end(); ++i) {
		Command &cmd = *i;
		switch(cmd.type) {
			case Command::Push: {
					assert(cmd.object != NULL);
					if (cmd.id < 0) {
						cmd.id = 1 + math::max((_objects.empty()? 0: _objects.rbegin()->first), _last_id);
						if (cmd.id > _last_id)
							_last_id = cmd.id;
					}

					assert(cmd.id > 0);
					on_object_replace_id.emit(cmd.object, cmd.id);
					cmd.object->_id = cmd.id;
					LOG_DEBUG(("pushing %d:%s", cmd.id, cmd.object->animation.c_str()));
					
					ObjectMap::iterator j = _objects.find(cmd.id);
					if (j != _objects.end()) {
						_grid.remove(j->second);
						delete j->second;
						j->second = cmd.object;
					} else {
						_objects.insert(ObjectMap::value_type(cmd.id, cmd.object));
					}
					updateObject(cmd.object);
				}
				break;
			case Command::Pop: {
					ObjectMap::iterator j = _objects.find(cmd.id);
					if (j != _objects.end()) {
						Object *o = j->second;
						_grid.remove(o);
						delete o;
						//if (o->_dead && !o->animation.empty())
						//	o->_dead = false;
						_objects.erase(j);
						objects.erase(cmd.id);
					}
				}
				break;
			default: 
				assert(0);
		}
		
	}
	_commands.clear();

	for(ObjectMap::iterator i = objects.begin(); i != objects.end(); ) {
		Object *o = i->second;
		assert(o != NULL);

		if (!PlayerManager->is_client() && o->_dead) { //not dead/dead and server mode
			//LOG_DEBUG(("object %d:%s is dead. cleaning up. (global map: %s)", o->get_id(), o->classname.c_str(), &objects == &_objects?"true":"false"));
			int id = i->first;
			deleteObject(o);
			o = NULL;
			objects.erase(i++);
			_objects.erase(id);
		} else {
			++i; //not dead / dead and safe mode. waiting for the update.
		}
	}
}


const bool IWorld::exists(const int id) const {
	return _objects.find(id) != _objects.end();
}

const Object *IWorld::getObjectByID(const int id) const {
	ObjectMap::const_iterator i = _objects.find(id);
	if (i != _objects.end() && !i->second->is_dead())
		return i->second;
	return NULL;
}

Object *IWorld::getObjectByID(const int id) {
	ObjectMap::iterator i = _objects.find(id);
	if (i != _objects.end())
		return i->second;
	return NULL;
}


Object* IWorld::spawn(const Object *src, const std::string &classname, const std::string &animation, const v2<float> &dpos, const v2<float> &vel, const int z) {
	Object *obj = ResourceManager->createObject(classname, animation);
	
	assert(obj->_owners.empty());
	
	obj->copy_owners(src);
	obj->set_slot(src->get_slot());
		
	obj->add_owner(src->_id);
	//LOG_DEBUG(("%s spawns %s", src->classname.c_str(), obj->classname.c_str()));
	obj->_spawned_by = src->_id;
	
	obj->_velocity = vel;
	
	//LOG_DEBUG(("spawning %s, position = %g %g dPosition = %g:%g, velocity: %g %g", 
	//	classname.c_str(), src->_position.x, src->_position.y, dpos.x, dpos.y, vel.x, vel.y));
	v2<float> pos = src->get_position() + (src->size / 2)+ dpos - (obj->size / 2);

	obj->_z -= ZBox::getBoxBase(obj->_z);
	obj->_z += ZBox::getBoxBase(src->_z);
	
	addObject(obj, pos);

	if (z) 
		obj->set_z(z);

	obj->_z -= ZBox::getBoxBase(obj->_z);
	obj->_z += ZBox::getBoxBase(src->_z);
	//LOG_DEBUG(("spawn: %s: %d, parent: %s, %d", obj->animation.c_str(), obj->_z, src->animation.c_str(), src->_z));

	return obj;
}

void IWorld::serializeObjectPV(mrt::Serializator &s, const Object *o) const {
	v2<float> pos = o->_position;

	if (o->_interpolation_progress < 1.0f) {
		Map->add(pos, o->_interpolation_vector * ( 1.0f - o->_interpolation_progress));
		s.add(pos);
	} else {
		Map->validate(pos);
		s.add(pos);
	}

	s.add(o->_velocity);
	s.add(o->get_z());
	s.add(o->_direction);
	s.add(o->_direction_idx);
}

void IWorld::deserializeObjectPV(const mrt::Serializator &s, Object *o) {
	int z;
	if (o == NULL) {
		v2<float> x;
		s.get(x);
		s.get(x);
		s.get(z);

		s.get(x); //direction
		s.get(z);

		LOG_WARN(("skipped deserializeObjectPV for NULL object"));
		return;
	}
	o->uninterpolate();
	o->_interpolation_position_backup = o->_position;
	
	s.get(o->_position);
	s.get(o->_velocity);
	s.get(z);
	if (!ZBox::sameBox(o->get_z(), z))
		o->set_zbox(z);

	s.get(o->_direction);
	s.get(o->_direction_idx);
}


void IWorld::serializeObject(mrt::Serializator &s, const Object *o, const bool force) const {
	if (o->_dead) {
		LOG_WARN(("%d:%s is dead, skipping object", o->_id, o->animation.c_str()));
		return;
	}
	s.add(o->_id);
	s.add(o->registered_name);
	if (force)
		o->serialize_all(s);
	else 
		o->serialize(s);
}


void IWorld::serialize(mrt::Serializator &s) const {
	s.add(_last_id);
	for(ObjectMap::const_iterator i = _objects.begin(); i != _objects.end(); ++i) {
		const Object *o = i->second;
		serializeObject(s, o, true);
	}
	s.add(0);

	GET_CONFIG_VALUE("engine.speed", float, e_speed, 1.0f);
	s.add(e_speed);
}

void IWorld::sync(const int id) {
	if (_out_of_sync == -1 || id < _out_of_sync) 
		_out_of_sync = id;
}

Object * IWorld::deserializeObject(const mrt::Serializator &s) {
	int id;
	std::string rn;
	Object *ao = NULL, *result = NULL;
	TRY {
		s.get(id);
		if (id <= 0)
			return NULL; //end of stream - to avoid needless estimate calculations
		if (id > _last_id) 
			_last_id = id;
		
		s.get(rn);
		{
			ObjectMap::iterator i = _objects.find(id);
			if (i != _objects.end()) {
				//object with given ID exists in map.
				Object *o = i->second;
				assert(o != NULL);
				assert(o->_id == id);
				
				if (rn == o->registered_name) {
					PlayerSlot * slot = PlayerManager->get_slot_by_id(id);
					if (slot == NULL) {
						o->deserialize(s);
						if (o->_dead) {
							LOG_DEBUG(("incomplete data for object %d:%s", o->_id, o->animation.c_str()));
							sync(o->_id);
						}
					} else { 
						//state, 
						PlayerState state = o->_state;
						v2<float> pos = o->_position, vel = o->_velocity, ipos_backup = o->_interpolation_position_backup;
						float ip = o->_interpolation_progress;
						
						o->deserialize(s);
						if (o->_dead) {
							LOG_DEBUG(("incomplete data for object %d:%s", o->_id, o->animation.c_str()));
							sync(o->_id);
						}
						
						o->_state = state;
						o->_position = pos;
						o->_velocity = vel;
						o->_interpolation_position_backup = ipos_backup;
						o->_interpolation_progress = ip;
					}
					
					result = o;
				} else {
					//object storage type differs from existing object
					ao = ResourceManager->createObject(rn);
					//LOG_DEBUG(("created ('%s', '%s')", rn.c_str(), an.c_str()));
					ao->deserialize(s);
					
					_grid.remove(o);
					delete o;
					o = NULL;
					i->second = ao;
					result = ao;
					ao = NULL;
					_grid.update(result, result->_position.convert<int>(), result->size.convert<int>());

					
					if (!result->_need_sync || result->_dead) {
						LOG_DEBUG(("incomplete data for object %d:%s", result->_id, result->animation.c_str()));
						result->_dead = true;
						sync(result->_id);
					}
				}
			} else {
				//new object.
				ao = ResourceManager->createObject(rn);
				//LOG_DEBUG(("created ('%s', '%s')", rn.c_str(), an.c_str()));
				
				ao->deserialize(s);
				assert(ao->_id == id);
				
				_objects[id] = ao;
				result = ao;
				ao = NULL;
				
				if (!result->_need_sync || result->_dead) {
					LOG_DEBUG(("incomplete data for object %d:%s", result->_id, rn.c_str()));
					result->_dead = true;
					sync(result->_id);
				}
			}

			//LOG_DEBUG(("deserialized %d: %s", ao->_id, ao->classname.c_str()));
		}
	} CATCH(mrt::format_string("deserializeObject('%d:%s')", id, rn.c_str()).c_str(), { 
			delete ao; throw; 
		})
	assert(result != NULL);
	assert(!result->animation.empty() || result->_dead);
	updateObject(result);
	//LOG_DEBUG(("deserialized object: %d:%s:%s", id, result->registered_name.c_str(), result->animation.c_str()));
	return result;
}

void IWorld::cropObjects(const std::set<int> &ids) {
	for(ObjectMap::iterator i = _objects.begin(); i != _objects.end(); /*haha*/ ) {
		Object *o = i->second;
		
		if (ids.find(i->first) == ids.end()) {
			deleteObject(o);
			_objects.erase(i++);
		} else {
			if (o->_dead && (_out_of_sync == -1 || o->_id < _out_of_sync) ) {
				if (o->animation.empty()) {
					LOG_WARN(("BUG: object %d is out of sync, double check out-of-sync code.", o->get_id()));
					sync(o->get_id());
				} else {
					LOG_DEBUG(("resurrecting object %d(%s) from the dead", o->get_id(), o->animation.c_str()));
					o->_dead = false;
				}
			}
			++i;
		}
	}
}

#include "var.h"

void IWorld::deserialize(const mrt::Serializator &s) {
TRY {
	s.get(_last_id);
	//_last_id += 10000;
	
	std::set<int> recv_ids;

	Object *obj;
	while((obj = deserializeObject(s)) != NULL) {
		recv_ids.insert(obj->_id);
	}
	cropObjects(recv_ids);
	float speed;
	s.get(speed);
	setSpeed(speed);
} CATCH("World::deserialize()", throw;);
	//LOG_DEBUG(("deserialization completed successfully"));
}

void IWorld::generateUpdate(mrt::Serializator &s, const bool clean_sync_flag, const int first_id) {
	GET_CONFIG_VALUE("multiplayer.sync-interval-divisor", int, sync_div, 5);
	
	const bool sync_update = first_id > 0;
	int id0 = sync_update? first_id: _current_update_id;
	
	int n = 0, max_n = _objects.size() / sync_div;
		
	std::priority_queue<Object *, std::vector<Object *>, BaseObject::PriorityComparator<Object *> > priority_objects;
	ObjectMap::iterator i;

	if (!sync_update) {
		//let's use distance stats to prioritize some objects: 
		int pc = 0, sum_w = 0, win_n = 0;
		for(i = _objects.begin(); i != _objects.end(); ++i) {
			Object *o = i->second;
			assert(o != NULL);
			if (o->_dead || o->registered_name == "damage-digits") 
				continue;
			
			int w = o->get_mp_priority();
			if (w > 0) {
				priority_objects.push(o);
				sum_w += w;
				++pc;
			}
		}
		while(!priority_objects.empty() && n < max_n) {
			Object *o = priority_objects.top();
			priority_objects.pop();

			int w = o->get_mp_priority();
			bool win = mrt::random(sum_w) < 2 * w;
			if (win) {
				++win_n;
				serializeObject(s, o, sync_update);
				if (clean_sync_flag)
					o->set_sync(false);
				++n;
			}
			//LOG_DEBUG(("%d: object %s with weight %d, freq: %g, win: %c", pc++, o->animation.c_str(), w, 1.0 * w / sum_w, win?'+':'-'));
			o->reset_mp_priority();
		}
		LOG_DEBUG(("priorities: %d, total weight: %d, total: %u, wins: %d", pc, sum_w, (unsigned)_objects.size(), win_n));	
	}
	
	for(i = _objects.lower_bound(id0); i != _objects.end() && i->first < id0; ++i);
	
	for( ; i != _objects.end() && (sync_update || n < max_n); ++i) {
		Object *o = i->second;
		assert(o != NULL);
		assert(o->_id >= id0);
		if (o->_dead) {
			LOG_WARN(("%d:%s is dead, skipping object", o->_id, o->animation.c_str()));
			continue;
		}

		//if (!sync_update && o->speed == 0 && !o->_need_sync) 
		//	continue; //actually this hack causing cannons to be out-of-sync everywhere :(

		serializeObject(s, o, sync_update);
		if (clean_sync_flag)
			o->set_sync(false);
		
		++n;
	}
	if (!sync_update) {
		if (i != _objects.end()) {
			_current_update_id = i->first;
		} else {
			_current_update_id = -1;
		}
	}
	
	{
		int dummy = 0;
		s.add(dummy); //end of stream marker
	}
	
	bool crop = i == _objects.end();
	s.add(crop);
	if (crop) {
		std::set<int> ids;
		for(ObjectMap::iterator i = _objects.begin(); i != _objects.end(); ++i) 
			ids.insert(i->first);
		s.add(ids);
	
		s.add(_last_id);
		
		GET_CONFIG_VALUE("engine.speed", float, e_speed, 1.0f);
		s.add(e_speed);
	}
	
	//LOG_DEBUG(("generated update: %d objects", n));
		
	mrt::random_serialize(s);
}

void IWorld::interpolateObject(Object *o) {
	GET_CONFIG_VALUE("multiplayer.disable-interpolation", bool, di, false);
	if (di)
		return;
	
	if (o->_interpolation_position_backup.is0()) //newly deserialized object
		return;

	GET_CONFIG_VALUE("multiplayer.maximum-interpolation-distance", float, mdd, 128.0f);
	
	const float distance = o->_position.distance(o->_interpolation_position_backup);
	if (distance < 1 || distance > mdd) {
		o->_interpolation_position_backup.clear();
		o->_interpolation_progress = 1.0f;
		return;
	}
			
	o->_interpolation_vector = Map->distance(o->_interpolation_position_backup, o->_position);
	o->_position = o->_interpolation_position_backup;
	o->_interpolation_position_backup.clear();
	o->_interpolation_progress = 0;
}

void IWorld::interpolateObjects(ObjectMap &objects) {
	GET_CONFIG_VALUE("multiplayer.disable-interpolation", bool, di, false);
	if (di)
		return;
	
	for(ObjectMap::iterator i = objects.begin(); i != objects.end(); ++i) {
		Object *o = i->second;
		assert(o != NULL);
		interpolateObject(o);
	}
}

void IWorld::applyUpdate(const mrt::Serializator &s, const float dt, const int sync_id) {
TRY {
	_collision_map.clear();
	if (sync_id > 0) {
		LOG_DEBUG(("catched update with 'sync=%d' flag set", sync_id));
		if (_out_of_sync >= sync_id) {
			_out_of_sync = -1;
			_out_of_sync_sent = -1;
		}
	}

	ObjectMap objects;
	Object *o;
	while((o = deserializeObject(s)) != NULL) {
		objects.insert(ObjectMap::value_type(o->_id, o));
		/*ObjectMap::iterator i = push_objects.find(o->_id);
		if (i != push_objects.end()) {
			delete i->second;
			push_objects.erase(i);
		}
		pop_objects.erase(o->_id);
		*/
	}
	std::set<int> ids;
	
	bool crop;
	s.get(crop); 
	
	if (crop) {
		s.get(ids);
		s.get(_last_id);

		float speed;
		s.get(speed);
		setSpeed(speed);
	}

	mrt::random_deserialize(s);

	//_last_id += 10000;
	TRY {
		if (crop)
			cropObjects(ids);
	} CATCH("applyUpdate::cropObjects", throw;);

	TRY {
		tick(objects, dt);
	} CATCH("applyUpdate::tick", throw;);
	
	interpolateObjects(objects);
	if (_out_of_sync != _out_of_sync_sent) {
		PlayerManager->request_objects(_out_of_sync);
		_out_of_sync_sent = _out_of_sync;
	}
	purge(objects, dt);
	purge(0);
} CATCH("applyUpdate", throw;)
}

#define PIERCEABLE_PAIR(o1, o2) ((o1->piercing && o2->pierceable) || (o2->piercing && o1->pierceable))

void IWorld::setSpeed(const float speed) {
	GET_CONFIG_VALUE("engine.speed", float, e_speed, 1.0f);
	if (speed != e_speed) {
		Var v;
		v.type = "float";
		v.f = speed;
		Config->setOverride("engine.speed", v);
		Config->invalidateCachedValues();
	}
}

const Object* IWorld::get_nearest_object(const Object *obj, const std::set<std::string> &classnames, const float range, const bool check_shooting_range) const {
	if (classnames.empty())
		return NULL;

	const Object *result = NULL;
	float distance = std::numeric_limits<float>::infinity();
	float range2 = range * range;

	v2<float> position = obj->get_center_position();
	std::set<Object *> objects;
	_grid.collide(objects, (position - range).convert<int>(), v2<int>((int)(range * 2), (int)(range * 2)));
	//consult grid

	for(std::set<Object *>::const_iterator i = objects.begin(); i != objects.end(); ++i) {
		Object *o = *i;
		//LOG_DEBUG(("%s is looking for %s. found: %s", obj->classname.c_str(), classname.c_str(), o->classname.c_str()));
		if (o->_id == obj->_id || o->impassability == 0 || PIERCEABLE_PAIR(obj, o) || !ZBox::sameBox(obj->get_z(), o->get_z()) ||
			classnames.find(o->classname) == classnames.end() || o->has_same_owner(obj))
			continue;

		if (check_shooting_range && !Object::check_distance(position, o->get_center_position(), o->get_z(), true))	
			continue;

		v2<float> dpos = Map->distance(o->get_center_position(), position);
		
		float d = dpos.quick_length();
		if (d < range2 && d < distance) {
			distance = d;
			result = o;
		}
	}
	return result;
}

const bool IWorld::get_nearest(const Object *obj, const std::set<std::string> &classnames, const float range, v2<float> &position, v2<float> &velocity, const bool check_shooting_range) const {
	const Object *target = get_nearest_object(obj, classnames, range, check_shooting_range);
	
	if (target == NULL) 
		return false;

	v2<float> pos = obj->get_center_position();

	position = Map->distance(pos, target->get_center_position());
	velocity = target->_velocity;
	velocity.normalize();
	velocity *= target->speed;
	
	return true;
}

const int IWorld::get_children(const int id, const std::string &classname) const {
	int c = 0;
	for(ObjectMap::const_iterator i = _objects.begin(); i != _objects.end(); ++i) {
		if (i->first != id && 
			(i->second->_spawned_by == id || i->second->has_owner(id)) &&
		   	(classname.empty() || (classname == i->second->classname))
		  ) 
			++c;
	}
	return c;
}

void IWorld::enumerate_objects(std::set<const Object *> &id_set, const Object *src, const float range, const std::set<std::string> *classfilter) {
	id_set.clear();

	if (classfilter != NULL && classfilter->empty())
		return;

	float r2 = range * range;
	
	std::set<Object *> objects;
	v2<float> position = src->get_position(), center_position = src->get_center_position();
	_grid.collide(objects, (position - range).convert<int>(), v2<int>((int)(range * 2), (int)(range * 2)));
	//consult grid

	for(std::set<Object *>::const_iterator i = objects.begin(); i != objects.end(); ++i) {
		Object *o = *i;
		
		v2<float> dpos = Map->distance(center_position, o->get_center_position());
		if (o->_id == src->_id || !ZBox::sameBox(src->get_z(), o->get_z()) || dpos.quick_length() > r2)
			continue;

		if (classfilter != NULL && classfilter->find(o->classname) == classfilter->end())
			continue;
		
		id_set.insert(o);
	}
}

const Object *IWorld::getObjectByXY(const int x, const int y) const {
	for(ObjectMap::const_iterator i = _objects.begin(); i != _objects.end(); ++i) {
		const Object *o = i->second;
		sdlx::Rect r((int)o->_position.x, (int)o->_position.y, (int)o->size.x, (int)o->size.y);
		if (r.in(x, y))
			return o;
	}
	return NULL;
}

void IWorld::move(const Object *object, const int x, const int y) {
	Object *o = const_cast<Object *>(object);
	if (o != NULL) {
		o->_position.x = x; 
		o->_position.y = y; 
		updateObject(o);
	}
}

void IWorld::onMapResize(int left, int right, int up, int down) {
	LOG_DEBUG(("reacting to the map resize event"));
	v2<int> map_size = Map->get_size();
	for(ObjectMap::iterator i = _objects.begin(); i != _objects.end(); ++i) {
		Object *o = i->second;
		assert(o != NULL);
		
		o->_position.x += left;
		o->_position.y += up;
		if (o->_position.x < 0)
			o->_position.x = 0;
		if (o->_position.y < 0)
			o->_position.y = 0;
		v2<float> rpos = o->_position + o->size;
		if (rpos.x > map_size.x)
			o->_position.x = map_size.x - o->size.x;
		if (rpos.y > map_size.y)
			o->_position.y = map_size.y - o->size.y;
		updateObject(o);
		TRY {
			GameItem &item = GameMonitor->find(o);
			item.position = o->_position.convert<int>();
			item.updateMapProperty();
		} CATCH("moving object",)
	}
}

void IWorld::teleport(Object *object, const v2<float> &position) {
	object->_position = position - object->size / 2;
	updateObject(object);
	object->add_effect("teleportation", 1);
}

void IWorld::push(Object *parent, Object *object, const v2<float> &dpos) {
	LOG_DEBUG(("push (%s, %s, (%+g, %+g))", parent->animation.c_str(), object->animation.c_str(), dpos.x, dpos.y));
	Command cmd(Command::Push);
	cmd.id = object->get_id();

	object->_position = parent->_position + dpos;
	object->_parent = NULL;
	
	Map->validate(object->_position);
	
	cmd.object = object;
	_commands.push_back(cmd);
}

void IWorld::push(const int id, Object *object, const v2<float> &pos) {
	LOG_DEBUG(("push (%d, %s, (%g,%g))", id, object->animation.c_str(), pos.x, pos.y));
	Command cmd(Command::Push);
	cmd.id = id;

	object->_position = pos;
	object->_parent = NULL;
	
	Map->validate(object->_position);
	
	cmd.object = object;
	_commands.push_back(cmd);
}

Object * IWorld::pop(Object *object) {
	LOG_DEBUG(("pop %d:%s:%s", object->_id, object->animation.c_str(), object->_dead?"true":"false"));
	Command cmd(Command::Pop);
	cmd.id = object->get_id();

	Object *r = NULL;
	
	for(Commands::reverse_iterator i = _commands.rbegin(); i != _commands.rend(); ++i) {
		if (i->id == cmd.id) {
			r = i->object;
			assert(r != NULL);
			break;
		}
	}
	
	if (r == NULL) {
		ObjectMap::iterator j = _objects.find(cmd.id);
		if (j == _objects.end())
			throw_ex(("popping non-existent object %d %s", cmd.id, object->animation.c_str()));
		r = j->second;
	}
	assert(r != NULL);

	Object *o = r->deep_clone();
	assert(o != NULL);

	r->_dead = true;
	o->_position.clear();

	_commands.push_back(cmd);
	return o;
}
