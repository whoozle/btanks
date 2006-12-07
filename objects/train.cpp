/* Battle Tanks Game
 * Copyright (C) 2006 Battle Tanks team
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
#include "resource_manager.h"
#include "mrt/random.h"
#include "tmx/map.h"

class Train : public Object {
public:
	Train() : Object("train"), _smoke(1.0, true) { setDirectionsNumber(1); }
	virtual Object * clone() const;
	virtual void onSpawn();
	virtual void calculate(const float dt);
	virtual void tick(const float dt);
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(dst_y);
		_smoke.serialize(s);
	}

	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(dst_y);
		_smoke.deserialize(s);
	}

private: 
	int dst_y;
	Alarm _smoke;
};

void Train::onSpawn() {
	play("move", true);
	v3<int> size = Map->getSize();
	dst_y = size.y - 1; //fixme. :)
}

void Train::emit(const std::string &event, BaseObject * emitter) {
	if (event == "death") {
		spawn("corpse", "dead-choo-choo-train", v3<float>::empty, v3<float>::empty);
	}
	Object::emit(event, emitter);
}

void Train::tick(const float dt) {
	Object::tick(dt);
	v3<int> pos;
	getPosition(pos);
	//LOG_DEBUG(("pos: %d dst: %d", pos.y, dst_y));
	if (pos.y  >= dst_y) { 
		LOG_DEBUG(("escaped!"));
		emit("death", NULL);
	}
	if (_smoke.tick(dt)) {
		spawn("train-smoke", "train-smoke", v3<float>::empty, v3<float>::empty);
	}
}

void Train::calculate(const float dt) {
	_state.down = true;
	Object::calculate(dt);
}

Object* Train::clone() const  {
	return new Train(*this);
}

REGISTER_OBJECT("choo-choo-train", Train, ());
