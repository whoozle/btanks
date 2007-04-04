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


class WatchTower : public DestructableObject {
public: 
	WatchTower(const std::string &object, const std::string &animation) : 
		DestructableObject("watchtower"), _object(object), _animation(animation) {
			_variants.add("make-pierceable");
			_variants.add("with-fire");
		}
	Object *clone() const { return new WatchTower(*this); }
	
	virtual void onSpawn() {
		if (_object == "top") {
			play("top", true);
			return;
		}
		
		DestructableObject::onSpawn();
		Object *o;
		add("machinegunner", o = spawnGrouped(_object, _animation, v2<float>(0, -12), Centered));
		o->setZ(getZ() + 1);
		add("top", o = spawnGrouped("watchtower-top", "watchtower", v2<float>(0, 0), Centered));
		o->setZ(getZ() + 2);
	}
	
	virtual void tick(const float dt) {
		DestructableObject::tick(dt);
		if (_broken) {
			remove("machinegunner");
			remove("top");
		}
	}
	
	virtual void emit(const std::string &event, Object * emitter) {
		if (_object == "top") {
			Object::emit(event, emitter);
			return;
		}
		DestructableObject::emit(event, emitter);
	}


	virtual void serialize(mrt::Serializator &s) const {
		DestructableObject::serialize(s);
		s.add(_object);
		s.add(_animation);
	}

	virtual void deserialize(const mrt::Serializator &s) {
		DestructableObject::deserialize(s);
		s.get(_object);
		s.get(_animation);
	}

	
private: 
	std::string _object, _animation;
};

REGISTER_OBJECT("watchtower-top", WatchTower, ("top", ""));
REGISTER_OBJECT("watchtower-with-machinegunner", WatchTower, ("machinegunner-in-watchtower", "machinegunner"));
REGISTER_OBJECT("watchtower-with-thrower", WatchTower, ("thrower-in-watchtower", "thrower"));
