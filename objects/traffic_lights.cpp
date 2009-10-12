
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

class TrafficLights : public Object {
public:
	TrafficLights() : Object("traffic-lights"), _idx(0), _broken(false) {}
	virtual void tick(const float dt);
	virtual Object * clone() const;
	virtual void add_damage(Object *from, const int hp, const bool emitDeath = true);
	virtual void on_spawn();

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


void TrafficLights::on_spawn() {
	play("red");
}

void TrafficLights::add_damage(Object *from, const int dhp, const bool emitDeath) {
	if (_broken)
		return;

	Object::add_damage(from, dhp, false);
	if (hp <= 0) {
		_broken = true;
		cancel_all();
		play("fade-out", false); 
		play("broken", true);
		pierceable = true;
	}
}

void TrafficLights::tick(const float dt) {
	Object::tick(dt);

	static const char *names[] = {"red", "flashing-red", "yellow", "green", "flashing-green", "yellow"};
	
	if (get_state().empty()) {
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
