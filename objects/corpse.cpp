
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

class Corpse : public Object {
public:
	Corpse(const int fc, const bool play_dead) : Object("corpse"), _fire_cycles(fc), _play_dead(play_dead) {}

	virtual Object * clone() const;
	virtual void tick(const float dt);
	virtual void onSpawn();

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

private: 
	int _fire_cycles;
	bool _play_dead;
};

void Corpse::tick(const float dt) {
	Object::tick(dt);
	if (getState().empty()) {	
		//LOG_DEBUG(("over"));
		emit("death", this);
	}
}

void Corpse::onSpawn() {
	//LOG_DEBUG(("single-pose: play('%s', %s)", _pose.c_str(), _repeat?"true":"false"));
	if (_variants.has("human-death")) {
		playRandomSound("human-death", false);
	} else if (_variants.has("zombie-death")) {
		playRandomSound("zombie-death", false);
	}
	
	if (_fire_cycles > 0) {
		play("fade-in", false);
		for(int i = 0; i < _fire_cycles; ++i)
			play("burn", false);
		play("fade-out", false);
	}
	if (_play_dead)
		play("dead", true);
}


Object* Corpse::clone() const  {
	return new Corpse(*this);
}

REGISTER_OBJECT("corpse", Corpse, (16, true));
REGISTER_OBJECT("impassable-corpse", Corpse, (16, true));
REGISTER_OBJECT("static-corpse", Corpse, (0, true));
REGISTER_OBJECT("fire", Corpse, (16, false));
