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

namespace sdlx {
class Surface;
class Rect;
}

class Object;
class WorldMap;

class IWorld {
public:
	DECLARE_SINGLETON(IWorld);
	
	void addObject(Object *);
	const bool getInfo(Object *, float &x, float &y, float &z, float &vx, float &vy, float &vz) const;
	
	void render(sdlx::Surface &surface, const sdlx::Rect &viewport);
	void tick(WorldMap &map, const float dt);
private:
	typedef std::set<Object *> ObjectSet;
	ObjectSet _objects;
};

SINGLETON(World, IWorld);

#endif
