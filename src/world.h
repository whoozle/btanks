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
#include "v3.h"
#include "mrt/serializable.h"

namespace sdlx {
class Surface;
class Rect;
}

class Object;
class WorldMap;

class IWorld : public mrt::Serializable {
public:
	DECLARE_SINGLETON(IWorld);
	
	void addObject(Object *, const v3<float> &pos);
	const bool exists(const Object *) const;
	const bool getInfo(const Object *, v3<float> &pos, v3<float> &vel) const;
	
	void render(sdlx::Surface &surface, const sdlx::Rect &viewport);
	void tick(WorldMap &map, const float dt);
	
	const Object * spawn(Object *src, const std::string &classname, const std::string &animation, const v3<float> &dpos, const v3<float> &vel);
	
	const bool getNearest(const Object *obj, const std::string &classname, v3<float> &position, v3<float> &velocity) const;
	const float getImpassability(Object *obj, const sdlx::Surface &surface, const v3<int> &position) const;

	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);
private:
	typedef std::set<Object *> ObjectSet;
	ObjectSet _objects;
};

SINGLETON(World, IWorld);

#endif
