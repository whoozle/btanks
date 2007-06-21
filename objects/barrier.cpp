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
#include "config.h"
#include "alarm.h"

class Barrier : public Object {
public:
	Barrier() : Object("barrier"), _toggle(true) { pierceable = true; }
	virtual Object * clone() const;
	virtual void onSpawn();
	virtual void calculate(const float dt);
	virtual void tick(const float dt);

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_toggle);
	}

	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_toggle);
	}

private: 
	Alarm _toggle;
};

void Barrier::onSpawn() {
	GET_CONFIG_VALUE("objects.barrier.toggle-interval", float, ti, 3.0f);
	_toggle.set(ti);
	play("closed", true);
}

void Barrier::calculate(const float dt) {
	if (_toggle.tick(dt))
		_state.fire = !_state.fire;
}

void Barrier::tick(const float dt) {
	Object::tick(dt);
	if (_state.fire && getState() == "closed") {
		cancelAll();
		play("opening", false);
		play("opened", true);
	}

	if (!_state.fire && getState() == "opened") {
		cancelAll();
		play("closing", false);
		play("closed", true);
	}
}

Object* Barrier::clone() const  {
	return new Barrier(*this);
}

REGISTER_OBJECT("barrier", Barrier, ());
