#ifndef __BT_WORLD_H__
#define __BT_WORLD_H__

/* Battle Tanks Game
 * Copyright (C) 2006-2007 Battle Tanks team
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

#include "mrt/singleton.h"
#include "mrt/serializable.h"

#include <set>
#include <map>

#include "math/v2.h"
#include "math/matrix.h"
#include "object_common.h"

#include "object_grid.h"

namespace sdlx {
class Surface;
class Rect;
}

namespace ai {
class Traits;
}

class Object;

class IWorld : public mrt::Serializable {
public:
	DECLARE_SINGLETON(IWorld);
	typedef std::map<const int, Object*> ObjectMap;
	
	void clear();
	~IWorld();
	IWorld();
	
	void setTimeSlice(const float ts);
	
	void addObject(Object *, const v2<float> &pos, const int id = -1);
	const bool exists(const int id) const;
	const Object *getObjectByID(const int id) const;
	Object *getObjectByID(const int id);
	
	void render(sdlx::Surface &surface, const sdlx::Rect &src, const sdlx::Rect &viewport);
	void tick(const float dt);
	
	Object * spawn(Object *src, const std::string &classname, const std::string &animation, const v2<float> &dpos, const v2<float> &vel, const int z = 0);
	Object * spawnGrouped(Object *src, const std::string &classname, const std::string &animation, const v2<float> &dpos, const GroupType type);

//the nearest objects
	const Object* getNearestObject(const Object *obj, const std::string &classname) const;
	const bool getNearest(const Object *obj, const std::string &classname, v2<float> &position, v2<float> &velocity, Way * way = NULL) const;

	const Object* getNearestObject(const Object *obj, const std::set<std::string> &classnames) const;
	const bool getNearest(const Object *obj, const std::set<std::string> &classnames, v2<float> &position, v2<float> &velocity) const;

	const Object* getNearestObject(const Object *obj, const std::set<std::string> &classnames, const float range) const;
	const bool getNearest(const Object *obj, const std::set<std::string> &classnames, const float range, v2<float> &position, v2<float> &velocity) const;
//end of the nearest

	const bool old_findPath(const Object *obj, const v2<float>& position, Way & way, const Object * dst = NULL) const;
	void getImpassabilityMatrix(Matrix<int> &matrix, const Object *src, const Object *dst) const;

	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);
	void generateUpdate(mrt::Serializator &s, const bool clean_sync_flag);
	void applyUpdate(const mrt::Serializator &s, const float dt);

	void serializeObject(mrt::Serializator &, const Object *) const;
	Object* deserializeObject(const mrt::Serializator &);
	void serializeObjectPV(mrt::Serializator &, const Object *) const;
	void deserializeObjectPV(const mrt::Serializator &, Object *);

	static void interpolateObjects(ObjectMap &objects);
	static void uninterpolate(Object *o);

	void tick(Object &o, const float dt);	
	void tick(ObjectMap &objects, const float dt);
	
	void setSafeMode(const bool safe_mode = true);

	const float getImpassability(Object *obj, const v2<int> &position, const Object **collided_with = NULL, const bool probe = false, const bool skip_moving = false) const;
	
	const int getChildren(const int id) const;
	void setMode(const std::string &mode, const bool value);
	
	const bool attachVehicle(Object *object, Object *vehicle);
	const bool detachVehicle(Object *object);

	//ai stuff:	
	const Object * findTarget(const Object *src, const std::set<std::string> &enemies, const std::set<std::string> &bonuses, ai::Traits &traits) const;
	void enumerateObjects(std::set<const Object *> &o_set, const Object *src, const float range, const std::set<std::string> *classfilter);

	void initMap();
	
private:
	void updateObject(const Object *o);
	void deleteObject(const Object *o);
	
	void replaceID(const int old_id, const int new_id);
	
	typedef std::map<const std::pair<int, int>, bool> CollisionMap;
	mutable CollisionMap _collision_map;
	mutable CollisionMap _static_collision_map;
	
	//void getImpassability2(float &old_pos_im, float &new_pos_im, Object *obj, const v2<int> &new_position, const Object **old_pos_collided_with = NULL) const;
	const bool collides(Object *obj, const v2<int> &position, Object *other, const bool probe = false) const;

	
	void cropObjects(const std::set<int> &ids);

	ObjectMap _objects;
	Grid _grid; //hopefully it'll be our saviour.
	int _last_id;
	bool _safe_mode, _atatat;
	float _max_dt;
};

SINGLETON(World, IWorld);

#endif
