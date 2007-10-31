
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
#include "alarm.h"
#include "config.h"
#include "mrt/random.h"
#include "ai/herd.h"
#include "special_owners.h"

class Kamikaze : public Object, public ai::Herd {
public:
	Kamikaze() : 
		Object("kamikaze"), _reaction(true) {}
	
	virtual void tick(const float dt);
	virtual void calculate(const float dt);

	virtual Object * clone() const;
	virtual void onSpawn();
	virtual void emit(const std::string &event, Object * emitter = NULL);
	void on_idle(const int range, const float dt);

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_reaction);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_reaction);
	}	
	virtual const int getComfortDistance(const Object *other) const;

private: 
	Alarm _reaction;
};

const int Kamikaze::getComfortDistance(const Object *other) const {
	GET_CONFIG_VALUE("objects.kamikaze.comfort-distance", int, cd, 80);
	return (other == NULL || other->classname == "trooper" || other->classname == "kamikaze")?cd:-1;
}

void Kamikaze::on_idle(const int range, const float dt) {
	ai::Herd::calculateV(_velocity, this, 0, range);
	_state.fire = false;
}


void Kamikaze::calculate(const float dt) {
	if (!_reaction.tick(dt))
		return;
	
	v2<float> vel;
	static std::set<std::string> targets;
	if (targets.empty()) {
		targets.insert("train");
		targets.insert("fighting-vehicle");
		targets.insert("trooper");
		targets.insert("monster");
	}
	
	GET_CONFIG_VALUE("objects.kamikaze.targeting-range", int, tt, 800);

	if (getNearest(targets, tt, _velocity, vel, false)) {
		quantizeVelocity();
	} else on_idle(tt, dt);

	GET_CONFIG_VALUE("objects.kamikaze.rotation-time", float, rt, 0.05);
	limitRotation(dt, rt, true, false);
	updateStateFromVelocity();	
}

void Kamikaze::tick(const float dt) {
	const std::string state = getState();
	if (_velocity.is0()) {
		if (state != "hold") {
			cancelAll();
			play("hold", true);
		}
	} else {
		if (state == "hold") {
			cancelAll();
			play("run", true);
		}		
	}
	Object::tick(dt);
}

void Kamikaze::onSpawn() {
	GET_CONFIG_VALUE("objects.kamikaze.reaction-time", float, rt, 0.1);
	mrt::randomize(rt, rt/10);
	_reaction.set(rt);
	play("hold", true);
}

void Kamikaze::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
		spawn("kamikaze-explosion", "kamikaze-explosion");
		Object::emit(event, emitter);
	} else if (event == "collision") {
		if (emitter == NULL || (emitter->classname != "fighting-vehicle" && emitter->classname != "train" && emitter->classname != "trooper")) {
			Object::emit(event, emitter);
			return;
		}
		
		emit("death", emitter);
	} else 
		Object::emit(event, emitter);
}


Object* Kamikaze::clone() const  {
	return new Kamikaze(*this);
}

REGISTER_OBJECT("kamikaze", Kamikaze, ());
