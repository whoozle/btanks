#ifndef __BT_WORLD_H__
#define __BT_WORLD_H__
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

#include "mrt/singleton.h"
#include <set>
#include <map>
#include "math/v3.h"
#include "math/matrix.h"
#include "mrt/serializable.h"
#include "object_common.h"

namespace sdlx {
class Surface;
class Rect;
}

class Object;

class IWorld : public mrt::Serializable {
public:
	DECLARE_SINGLETON(IWorld);
	
	void clear();
	~IWorld();
	IWorld();
	
	void addObject(Object *, const v3<float> &pos);
	const bool exists(const Object *) const;
	const Object *getObjectByID(const int id) const;
	Object *getObjectByID(const int id);
	const bool getInfo(const Object *, v3<float> &pos, v3<float> &vel) const;
	
	void render(sdlx::Surface &surface, const sdlx::Rect &viewport);
	void tick(const float dt);
	
	Object * spawn(Object *src, const std::string &classname, const std::string &animation, const v3<float> &dpos, const v3<float> &vel);
	Object * spawnGrouped(Object *src, const std::string &classname, const std::string &animation, const v3<float> &dpos, const GroupType type);
	
	const bool getNearest(const Object *obj, const std::string &classname, v3<float> &position, v3<float> &velocity, Way * way = NULL) const;
	const float getImpassability(Object *obj, const sdlx::Surface &surface, const v3<int> &position, const Object **collided_with = NULL) const;
	void getImpassabilityMatrix(Matrix<int> &matrix, const Object *src, const Object *dst) const;

	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);
	void generateUpdate(mrt::Serializator &s);
	void applyUpdate(const mrt::Serializator &s, const int ping);
private:
	void serializeObject(mrt::Serializator &, const Object *) const;
	Object* deserializeObject(const mrt::Serializator &);
	
	void cropObjects(const std::set<int> &ids);

	typedef std::set<Object *> ObjectSet;
	typedef std::map<const int, Object*> ObjectMap;
	
	ObjectSet _objects;
	ObjectMap _id2obj;
	int _last_id;
};

SINGLETON(World, IWorld);

#endif
