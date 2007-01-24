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
#include "config.h"
#include "resource_manager.h"
#include "world.h"

class Barrack : public Object {
public:
	Barrack(const std::string &object, const std::string &animation) : 
		Object("barrack"), 
		_broken(false), _object(object), _animation(animation), _spawn(true) {}

	virtual Object * clone() const;
	virtual void tick(const float dt);
	virtual void onSpawn();
	virtual void addDamage(Object *from, const int hp, const bool emitDeath = true);

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_broken);
		s.add(_object);
		s.add(_animation);
	}

	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_broken);
		s.get(_object);
		s.get(_animation);
	}

private:
	bool _broken;
	std::string _object, _animation;
	Alarm _spawn;
};

void Barrack::addDamage(Object *from, const int dhp, const bool emitDeath) {
	if (_broken)
		return;

	Object::addDamage(from, dhp, false);
	if (hp <= 0) {
		_broken = true;
		pierceable = true;

		cancelAll();

		//play("fade-out", false); 
		play("broken", true);		
		spawn("fire", "fire", v3<float>::empty, v3<float>::empty);
	}
}

void Barrack::tick(const float dt) {
	Object::tick(dt);
	if (getState().empty()) {
		//LOG_DEBUG(("over"));
		emit("death", this);
	}
	if (_spawn.tick(dt)) {
		int max_c;
		Config->get("objects." + registered_name + ".maximum-children", max_c, 5);
		int n = World->getChildren(getID());
		if (n < max_c) {
			v3<float>dpos;
			dpos.y = size.y;
			
			Object *o = spawn(_object, _animation, dpos);
		}
	}
}

void Barrack::onSpawn() {
	play("main", true);
	float sr;
	Config->get("objects." + registered_name + ".spawn-rate", sr, 5);
	_spawn.set(sr);
}


Object* Barrack::clone() const  {
	return new Barrack(*this);
}

REGISTER_OBJECT("barrack-with-machinegunners", Barrack, ("machinegunner", "machinegunner"));
REGISTER_OBJECT("barrack-with-throwers", Barrack, ("thrower", "thrower"));
REGISTER_OBJECT("barrack-with-kamikazes", Barrack, ("kamikaze", "kamikaze"));

