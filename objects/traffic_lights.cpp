
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

class TrafficLights : public Object {
public:
	TrafficLights() : Object("traffic-lights"), _idx(0), _broken(false) {}
	virtual void tick(const float dt);
	virtual Object * clone() const;
	virtual void addDamage(Object *from, const int hp, const bool emitDeath = true);
	virtual void onSpawn();

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_idx);
		s.add(_broken);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_idx);
		s.get(_broken);
	}

private: 
	int _idx;
	bool _broken;
};


void TrafficLights::onSpawn() {
	play("red");
}

void TrafficLights::addDamage(Object *from, const int dhp, const bool emitDeath) {
	if (_broken)
		return;

	Object::addDamage(from, dhp, false);
	if (hp <= 0) {
		_broken = true;
		cancelAll();
		play("fade-out", false); 
		play("broken", true);
		pierceable = true;
	}
}

void TrafficLights::tick(const float dt) {
	Object::tick(dt);

	static const char *names[] = {"red", "flashing-red", "yellow", "green", "flashing-green", "yellow"};
	
	if (getState().empty()) {
		++_idx; 
		_idx %= sizeof(names) / sizeof(names[0]);

		//LOG_DEBUG(("tick! %d: %s", _idx, names[_idx]));
		play(names[_idx]);
	}
}

Object* TrafficLights::clone() const  {
	return new TrafficLights(*this);
}

REGISTER_OBJECT("traffic-lights", TrafficLights, ());
