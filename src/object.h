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
	v3<int> size;
	float mass, speed, ttl, impassability;
	int hp;
	
	bool piercing;
	
	const std::string classname;
	
	Object(const std::string &classname);
	virtual ~Object();
	
	virtual void tick(const float dt) = 0;
	virtual void render(sdlx::Surface &surf, const int x, const int y) = 0;
	virtual Object * clone(const std::string &opt) const = 0;
	virtual void emit(const std::string &event, const Object * emitter = NULL);
	
	const float getCollisionTime(const v3<float> &pos, const v3<float> &vel) const;
	
	void getPosition(v3<float> &position);
	const bool isDead() const;
protected:
	const Object * spawn(const std::string &classname, const std::string &animation, const v3<float> &dpos, const v3<float> &vel);
	const bool getNearest(const std::string &classname, v3<float> &position, v3<float> &velocity) const;

	
	v3<float> _velocity, _old_velocity, _direction;
private:
	bool _dead;
	v3<float> _position;
	Object *_owner;
	friend class IWorld;
};

#endif

