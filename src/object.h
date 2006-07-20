#ifndef __WORLD_OBJECT_H__
#define __WORLD_OBJECT_H__
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
#include <string>
#include "v3.h"
 
namespace sdlx {
	class Surface;
}

class Object {
public:
	float mass, w, h, speed, ttl;
	bool piercing;
	
	Object();
	virtual ~Object();
	
	virtual void tick(const float dt) = 0;
	virtual void render(sdlx::Surface &surf, const int x, const int y) = 0;
	virtual void emit(const std::string &event, const Object * emitter = NULL);
	
	void getPosition(v3<float> &position);
protected:
	void spawn(Object *, const v3<float> &dpos, const v3<float> &vel);
	v3<float> _velocity, _old_velocity;
private:
	bool dead;
	v3<float> _position;
	friend class IWorld;
};

#endif

