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

#include "object.h"
#include "alarm.h"
#include "registrar.h"
#include "mrt/random.h"

class Submarine : public Object {
public:
	Submarine() : Object("submarine"), _wakeup(false) { impassability = 0; hp = -1; }
	virtual Object * clone() const;
	virtual void onSpawn();
	virtual void tick(const float dt);

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_wakeup);
	}

	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_wakeup);
	}
	
	bool spawnBallistic();

protected:
	Alarm _wakeup;
};

void Submarine::onSpawn() {
	play("hold", true);
	_wakeup.set(mrt::random(5) + 5);
	playSound("submarine", true);
}


bool Submarine::spawnBallistic() {
	std::set<std::string> targets;
	
	targets.insert("fighting-vehicle");
	targets.insert("trooper");
	targets.insert("kamikaze");
	targets.insert("boat");
	targets.insert("helicopter");
	targets.insert("monster");
	targets.insert("watchtower");
	
	v2<float> pos, vel;
	if (getNearest(targets, 640.0f, pos, vel, false)) {
		spawn("ballistic-missile", "nuke-missile");
		return true;
	}
	return false;
}

void Submarine::tick(const float dt) {
	Object::tick(dt);

	if (getState().empty()) {
		_wakeup.set(mrt::random(5) + 5);
		play("hold", true);
	}
	if (_wakeup.tick(dt)) {
		//LOG_DEBUG(("waking up..."));
		spawnBallistic();
		_wakeup.set(3600);
		
		cancelAll();
		play("fade-in", false);
		int n = mrt::random(3) + 3;
		for(int i = 0; i < n; ++i) {
			play("main", false);
		}

		play("fade-out", false);
	}
}

Object* Submarine::clone() const  {
	return new Submarine(*this);
}

REGISTER_OBJECT("submarine", Submarine, ());
