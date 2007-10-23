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

#include "config.h"
#include "object.h"
#include "resource_manager.h"
#include "alarm.h"
#include "mrt/random.h"
#include "tmx/map.h"

class Boat : public Object {
public:
	Boat(const std::string &object);

	virtual Object* clone() const  { return new Boat(*this); }
	
	virtual void calculate(const float dt);
	virtual void tick(const float dt);
	virtual void onSpawn();
	void emit(const std::string &event, Object * emitter);

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_object);
		s.add(_fire);
		s.add(_reload);
		s.add(_reaction);
	}

	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_object);
		s.get(_fire);
		s.get(_reload);
		s.get(_reaction);
	}

private:
	std::string _object;
	Alarm _fire, _reload, _reaction;
};

void Boat::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
		spawn("corpse", "dead-" + animation);
	}
	Object::emit(event, emitter);
}


void Boat::calculate(const float dt) {
	if (!_reaction.tick(dt)) {
		calculateWayVelocity();

		GET_CONFIG_VALUE("objects." + registered_name + ".rotation-time", float, rt, 0.1f);
		limitRotation(dt, rt, true, false);

		return;
	}
	
	int tr;
	Config->get("objects." + registered_name + ".targeting-range", tr, 800);
	
	static std::set<std::string> targets;
	if (targets.empty()) {
		targets.insert("fighting-vehicle");
		//targets.insert("trooper");
		//targets.insert("kamikaze");
	}

	v2<float> pos, vel;
	if (getNearest(targets, tr, pos, vel, true)) {
		_state.fire = true;
	} else _state.fire = false;
	
	if (!isDriven()) {
		LOG_DEBUG(("finding next target..."));
		_velocity.clear();
		
		const v2<int> tile_size = Map->getPathTileSize();
		const Matrix<int> & water = Map->getAreaMatrix("water");
		v2<int> pos;
		getCenterPosition(pos);
		int im = water.get(pos.y / tile_size.y, pos.x / tile_size.x);
		if (im != 1) {
			emit("death", NULL); //bam! 
			return;
		}
		int dirs = getDirectionsNumber();
		int dir = mrt::random(dirs);
		v2<float> d; 
		d.fromDirection(dir, dirs);
		d.normalize((tile_size.x + tile_size.y) / 2);
		int len = 0;
		while(water.get(pos.y / tile_size.y, pos.x / tile_size.x) == 1) {
			++len;
			pos += d.convert<int>();
		}
		//LOG_DEBUG(("d: %g %g, len: %d", d.x, d.y, len));
		len -= (int)(size.x + size.y) / (tile_size.x + tile_size.y) / 2 + 1;
		if (len > 0) {
			len = 1 + len / 2 + (len % 2) + mrt::random(len / 2);
			getCenterPosition(pos);
			pos += (d * len).convert<int>();
			Way way;
			way.push_back(pos);
			setWay(way);
		}
	} 
	
	calculateWayVelocity();

	GET_CONFIG_VALUE("objects." + registered_name + ".rotation-time", float, rt, 0.1f);
	limitRotation(dt, rt, true, false);
}

void Boat::tick(const float dt) {
	Object::tick(dt);

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
	play("main", true);
	
	GET_CONFIG_VALUE("objects.missile-boat.fire-rate", float, fr, 0.5);
	_fire.set(fr);
	GET_CONFIG_VALUE("objects.missile-boat.reload-rate", float, rl, 3);
	_reload.set(rl);
	GET_CONFIG_VALUE("objects.missile-boat.reaction-time", float, rt, 0.15);
	mrt::randomize(rt, rt/10);
	_reaction.set(rt);
	
	Object *o = add("mod", "missiles-on-boat", "guided-missiles-on-launcher", v2<float>(), Centered);
	o->setZ(getZ() + 1, true);
}

Boat::Boat(const std::string &object) : Object("boat"), 
	_object(object), 
	_fire(false), 
	_reload(false), 
	_reaction(true) {
	setDirectionsNumber(8);
}


REGISTER_OBJECT("boat", Boat, ("guided"));
