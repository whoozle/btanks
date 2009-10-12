
/* Battle Tanks Game
 * Copyright (C) 2006-2009 Battle Tanks team
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

#include "object.h"
#include "registrar.h"

class Paratrooper : public Object {
public:
	Paratrooper(const std::string &classname, const std::string &spawn_object, const std::string &spawn_animation) : 
		Object(classname), _spawn_object(spawn_object), _spawn_animation(spawn_animation) { 
	}
	virtual void tick(const float dt);
	virtual Object * clone() const;
	virtual void on_spawn();
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
	if (get_state().empty()) {	
		//LOG_DEBUG(("over"));
		Object *kamikaze = spawn(_spawn_object, _spawn_animation);
		kamikaze->set_zbox(0);
		emit("death", this);
	}
}

void Paratrooper::on_spawn() {
	set_direction(0);
	play("main", false);
}

Object* Paratrooper::clone() const  {
	Object *a = new Paratrooper(*this);
	return a;
}

REGISTER_OBJECT("paratrooper-kamikaze", Paratrooper, ("paratrooper", "kamikaze", "kamikaze"));
REGISTER_OBJECT("paratrooper-machinegunner", Paratrooper, ("paratrooper", "machinegunner", "machinegunner"));
REGISTER_OBJECT("paratrooper-thrower", Paratrooper, ("paratrooper", "thrower", "thrower"));
