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

#include "object.h"
#include "resource_manager.h"
#include "config.h"

class Bomb : public Object {
public:
	Bomb() : Object("bomb"), z1(0), z2(0) { piercing = true; pierceable = true; }
	virtual Object * clone() const;
	virtual void onSpawn();
	virtual void calculate(const float dt);
	virtual void tick(const float dt);
	virtual void emit(const std::string &event, Object * emitter = NULL);

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(z1);
		s.add(z2);
	}
	
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(z1);
		s.get(z2);
	}
	
private: 
	int z1, z2;
};

void Bomb::onSpawn() {
	play("main", false);
	z1 = getZ();
	GET_CONFIG_VALUE("objects.bomb.lowest-z", int, z, 10);
	z2 = z;
}

void Bomb::calculate(const float dt) {
	_velocity.x = 0;
	_velocity.y = 1;
}

void Bomb::tick(const float dt) {
	Object::tick(dt);
	if (getState().empty())
		emit("death", this);
	int z = (int)(getStateProgress() * (z2 - z1)  + z1);
	//LOG_DEBUG(("setting z = %d", z));
	setZ(z, true);
}

void Bomb::emit(const std::string &event, Object * emitter) {
	if (event == "collision") {
		if (getStateProgress() >= 0.8) 
			emit("death", emitter);
		return; //do not emit addDamage
	} else if (event == "death") {
		spawn("cannon-explosion", "cannon-explosion");
	}
	Object::emit(event, emitter);
}


Object* Bomb::clone() const  {
	return new Bomb(*this);
}

REGISTER_OBJECT("bomb", Bomb, ());
