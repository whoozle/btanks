/* Battle Tanks Game
 * Copyright (C) 2006-2008 Battle Tanks team
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

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
*/

#include "destructable_object.h"
#include "registrar.h"


class WatchTower : public DestructableObject {
public: 
	WatchTower(const std::string &object, const std::string &animation) : 
		DestructableObject("watchtower"), _object(object), _animation(animation) {
			_variants.add("make-pierceable");
			_variants.add("with-fire");
		}
		
		
	Object *clone() const { return new WatchTower(*this); }
	
	virtual void on_spawn() {
		if (_object == "top") {
			play("top", true);
			return;
		}
		if (_variants.has("trainophobic"))
			_object += "(trainophobic)";

		DestructableObject::on_spawn();
		Object *o = add("machinegunner", _object, _animation, v2<float>(0, -12), Centered);
		o->set_z(get_z() + 1);
		o = add("top", "watchtower-top", "watchtower", v2<float>(0, 0), Centered);
		o->set_z(get_z() + 2);
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
