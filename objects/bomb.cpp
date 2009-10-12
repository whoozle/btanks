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
#include "config.h"

class Bomb : public Object {
public:
	Bomb() : Object("bomb"), z1(0), z2(0) { piercing = true; pierceable = true; }
	virtual Object * clone() const;
	virtual void on_spawn();
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

void Bomb::on_spawn() {
	play("main", false);
	z1 = get_z();
	GET_CONFIG_VALUE("objects.bomb.lowest-z", int, z, 610);
	z2 = z;
}

void Bomb::calculate(const float dt) {
	_velocity.x = 0;
	_velocity.y = 1;
}

void Bomb::tick(const float dt) {
	Object::tick(dt);
	if (get_state().empty())
		emit("death", this);
	int z = (int)(get_state_progress() * (z2 - z1)  + z1);
	//LOG_DEBUG(("setting z = %d [%d-%d]", z, z1, z2));
	set_z(z, true);
}

void Bomb::emit(const std::string &event, Object * emitter) {
	if (event == "collision") {
		if (emitter == NULL || get_state_progress() >= 0.8) 
			emit("death", emitter);
		return; //do not emit add_damage
	} else if (event == "death") {
		Object *o = spawn("cannon-explosion", "cannon-explosion");
		o->set_z(get_z() + 1, true);
	}
	Object::emit(event, emitter);
}


Object* Bomb::clone() const  {
	return new Bomb(*this);
}

REGISTER_OBJECT("bomb", Bomb, ());
