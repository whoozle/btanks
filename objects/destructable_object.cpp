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

#include "destructable_object.h"
#include "resource_manager.h"
#include "world.h"

DestructableObject::DestructableObject(const std::string &classname) : 
		Object(classname), 
		_broken(false),
		_object_id(0) {}

void DestructableObject::serialize(mrt::Serializator &s) const {
	Object::serialize(s);
	s.add(_broken);
	s.add(_object_id);
}

void DestructableObject::deserialize(const mrt::Serializator &s) {
	Object::deserialize(s);
	s.get(_broken);
	s.get(_object_id);
}

void DestructableObject::onBreak() {
}


void DestructableObject::addDamage(Object *from, const int dhp, const bool emitDeath) {
	if (_broken)
		return;

	Object::addDamage(from, dhp, false);
	if (hp <= 0) {
		_broken = true;
		hp = -1;
		if (_variants.has("make-pierceable"))
			pierceable = true;
		cancelAll();
		play("fade-out", false); 
		play("broken", true);
		classname = "debris";
		
		if (_variants.has("with-fire")) {
			Object *o = spawn("fire", "fire", v2<float>::empty, v2<float>::empty, getZ() + 1);
			_object_id = o->getID();			
		}
		onBreak();
	}
}

void DestructableObject::tick(const float dt) {
	Object::tick(dt);
	const std::string& state = getState();
	if (_object_id && state == "broken") {
		Object *o = World->getObjectByID(_object_id);
		if (o != NULL)
			o->setZ(getZ() + 1);
		_object_id = 0;
	}
	if (state.empty()) {	
		//LOG_DEBUG(("over"));
		emit("death", this);
	}
}

void DestructableObject::onSpawn() {
	play("main", true);
}


Object* DestructableObject::clone() const  {
	return new DestructableObject(*this);
}

REGISTER_OBJECT("destructable-object", DestructableObject, ("destructable-object"));
