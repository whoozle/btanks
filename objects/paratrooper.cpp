
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

class Paratrooper : public Object {
public:
	Paratrooper(const std::string &classname, const std::string &spawn_object, const std::string &spawn_animation) : 
		Object(classname), _spawn_object(spawn_object), _spawn_animation(spawn_animation) { 
	}
	virtual void tick(const float dt);
	virtual Object * clone() const;
	virtual void onSpawn();
	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_spawn_object);
		s.add(_spawn_animation);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_spawn_object);
		s.get(_spawn_animation);
	}
private:
	std::string _spawn_object, _spawn_animation;
};


void Paratrooper::tick(const float dt) {
	Object::tick(dt);
	if (getState().empty()) {	
		//LOG_DEBUG(("over"));
		Object *o = spawn(_spawn_object, _spawn_animation);
		o->setOwner(getOwner());
		emit("death", this);
	}
}

void Paratrooper::onSpawn() {
	setDirection(0);
	play("main", false);
}

Object* Paratrooper::clone() const  {
	Object *a = new Paratrooper(*this);
	return a;
}

REGISTER_OBJECT("paratrooper-kamikaze", Paratrooper, ("paratrooper", "kamikaze", "kamikaze"));
