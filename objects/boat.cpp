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

#include "destructable_object.h"
#include "config.h"
#include "resource_manager.h"
#include "world.h"
#include "alarm.h"
#include "mrt/random.h"

class Boat : public DestructableObject {
public:
	Boat(const std::string &object);

	virtual Object* clone() const  { return new Boat(*this); }
	
	virtual void calculate(const float dt);
	virtual void tick(const float dt);
	virtual void onSpawn();

	virtual void serialize(mrt::Serializator &s) const {
		DestructableObject::serialize(s);
		s.add(_object);
		_fire.serialize(s);
		_reload.serialize(s);
		_reaction.serialize(s);
	}

	virtual void deserialize(const mrt::Serializator &s) {
		DestructableObject::deserialize(s);
		s.get(_object);
		_fire.deserialize(s);
		_reload.deserialize(s);
		_reaction.deserialize(s);
	}

private:
	std::string _object;
	Alarm _fire, _reload, _reaction;
};

void Boat::calculate(const float dt) {
	if (!_reaction.tick(dt)) 
		return;
	
	GET_CONFIG_VALUE("objects.missile-boat.targeting-range", int, tr, 800);
	
	static std::set<std::string> targets;
	if (targets.empty()) {
		targets.insert("player");
		//targets.insert("trooper");
		//targets.insert("kamikaze");
	}

	v2<float> pos, vel;
	if (getNearest(targets, tr, pos, vel)) {
		_state.fire = true;
	} else _state.fire = false;
}

void Boat::tick(const float dt) {
	DestructableObject::tick(dt);
	if (_broken) {
		remove("mod");
		return;
	}
	const std::string state = getState();
	if (state == "reload" && _reload.tick(dt)) {
		_reload.reset();
		cancelAll();
		groupEmit("mod", "reload");
		play("main", true);
	}
	
	bool can_fire = _fire.tick(dt);
	if (_state.fire && can_fire && state != "reload") {
		_fire.reset();
		groupEmit("mod", "launch");
		if (get("mod")->getCount() == 0) {
			cancelRepeatable();
			play("reload", true);
		}
	}
}

void Boat::onSpawn() {
	DestructableObject::onSpawn();
	
	GET_CONFIG_VALUE("objects.missile-boat.fire-rate", float, fr, 0.5);
	_fire.set(fr);
	GET_CONFIG_VALUE("objects.missile-boat.reload-rate", float, rl, 3);
	_reload.set(rl);
	GET_CONFIG_VALUE("objects.missile-boat.reaction-time", float, rt, 0.15);
	mrt::randomize(rt, rt/10);
	_reaction.set(rt);
	
	add("mod", spawnGrouped("missiles-on-boat", "guided-missiles-on-launcher", v2<float>(size.x/3, 10), Centered));

}

Boat::Boat(const std::string &object) : 
	DestructableObject("boat"), 
	_object(object), 
	_fire(false), 
	_reload(false), 
	_reaction(true) {
	_variants.add("with-fire");
}


REGISTER_OBJECT("missile-boat", Boat, ("guided"));
