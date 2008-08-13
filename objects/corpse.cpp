
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

#include "object.h"
#include "registrar.h"

class Corpse : public Object {
public:
	Corpse(const int fc, const bool play_dead) : Object("corpse"), _fire_cycles(fc), _play_dead(play_dead) {}

	virtual Object * clone() const;
	virtual void tick(const float dt);
	virtual void on_spawn();

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_fire_cycles);
		s.add(_play_dead);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_fire_cycles);
		s.get(_play_dead);
	}
	
	void emit(const std::string &event, Object * emitter) {
		if (emitter != NULL && _variants.has("do-damage") && event == "collision" && emitter->classname != "corpse") {
			if (get_state() == "burn" || get_state() == "fade-out") {
				if (hp > 0)
					emitter->add_damage(this, emitter->hp);
			}
		}
		Object::emit(event, emitter);
	}

private: 
	int _fire_cycles;
	bool _play_dead;
};

void Corpse::tick(const float dt) {
	Object::tick(dt);
	if (get_state().empty()) {	
		//LOG_DEBUG(("over"));
		emit("death", this);
		return;
	}
	if (_variants.has("with-fire") && !has("fire") && (get_state() == "burn" || get_state() == "fade-out")) {
		Object *o = add("fire", "fire", "fire", v2<float>(), Centered);
		o->set_z(get_z() + 1, true);
	}
}

void Corpse::on_spawn() {
	//LOG_DEBUG(("single-pose: play('%s', %s)", _pose.c_str(), _repeat?"true":"false"));
	if (_variants.has("human-death")) {
		play_random_sound("human-death", false);
	} else if (_variants.has("zombie-death")) {
		play_sound("zombie-dead", false);
	} else if (_variants.has("slime-death")) {
		play_sound("slime-dead", false);
	}
	
	if (_fire_cycles > 0) {
		play("fade-in", false);
		for(int i = 0; i < _fire_cycles; ++i)
			play("burn", false);
		play("fade-out", false);
	}
	if (_play_dead)
		play("dead", true);
	if (get_state().empty())
		throw_ex(("corpse w/o state!"));
}


Object* Corpse::clone() const  {
	return new Corpse(*this);
}

REGISTER_OBJECT("corpse", Corpse, (16, true));
REGISTER_OBJECT("impassable-corpse", Corpse, (16, true));

REGISTER_OBJECT("fire", Corpse, (16, false));

REGISTER_OBJECT("impassable-static-corpse", Corpse, (0, true));
REGISTER_OBJECT("static-corpse", Corpse, (0, true));
