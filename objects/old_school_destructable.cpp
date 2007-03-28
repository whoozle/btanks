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
#include "object.h"

class OldSchoolDestructableObject : public Object {
public:
	OldSchoolDestructableObject(const int hops, const bool make_pierceable, const std::string &message = std::string());

	virtual Object * clone() const { return new OldSchoolDestructableObject(*this); }
	virtual void tick(const float dt);
	virtual void onSpawn();
	virtual void addDamage(Object *from, const int hp, const bool emitDeath = true);

	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);

protected:
	int _hops;
	int _explosions;

private: 
	bool _make_pierceable;
	std::string _message;
	Alarm _spawn;
};

#include "mrt/random.h"
#include "config.h"


OldSchoolDestructableObject::OldSchoolDestructableObject(const int hops, const bool make_pierceable, const std::string &message) : 
		Object("destructable-object"), 
		_hops(hops),
		_explosions(0), 
		_make_pierceable(make_pierceable), _message(message), _spawn(true) {}

void OldSchoolDestructableObject::serialize(mrt::Serializator &s) const {
	Object::serialize(s);
	s.add(_hops);
	s.add(_explosions);
	s.add(_make_pierceable);
	s.add(_message);
	_spawn.serialize(s);
}

void OldSchoolDestructableObject::deserialize(const mrt::Serializator &s) {
	Object::deserialize(s);
	s.get(_hops);
	s.get(_explosions);
	s.get(_make_pierceable);
	s.get(_message);
	_spawn.deserialize(s);
}

void OldSchoolDestructableObject::addDamage(Object *from, const int dhp, const bool emitDeath) {
	if (_hops <= 0)
		return;

	Object::addDamage(from, dhp, false);
	if (hp <= 0) {
		Config->get("objects." + registered_name + ".explosions", _explosions, 16);		
		hp = -1;
	}
}

#include "game.h"

void OldSchoolDestructableObject::tick(const float dt) {
	Object::tick(dt);
	
	if (!_spawn.tick(dt))
		return;
	
	if (!_message.empty()) {
		int dr;
		Config->get("objects." + registered_name + ".display-range", dr, 400);
		v2<float> pos, vel;
		static std::set<std::string> targets;
		if (targets.empty()) 
			targets.insert("player");
		
		if (getNearest(targets, dr, pos, vel)) {
			Game->displayMessage("messages", _message, 5);
			_message.clear();
		}
	}
		
	if (_explosions != 0) {
		int e;
		Config->get("objects." + registered_name + ".explosions", e, 16);		
		
		if (_explosions == (e + 1)/2) {
			--_hops;

			cancelAll();

			if (_hops == 0) {
				//completely dead
				if (_make_pierceable)
					pierceable = true;
				hp = -1; 
				play("broken", true);
			} else {
				hp = max_hp;
				play(mrt::formatString("damaged-%d", _hops), true);
			}
		}
		
		v2<float> dpos; 
		dpos.x = mrt::random((int)size.x) - size.x / 2;
		dpos.y = mrt::random((int)size.y) - size.y / 2;
		
		spawn("explosion", "building-explosion", dpos);
		--_explosions;
	}
}

void OldSchoolDestructableObject::onSpawn() {
	float er; 
	Config->get("objects." + registered_name + ".explosion-rate", er, 0.1);
	_spawn.set(er);
	play("main", true);
}

REGISTER_OBJECT("old-school-destructable-object-2", OldSchoolDestructableObject, (2, false));
REGISTER_OBJECT("spaceport-baykonur", OldSchoolDestructableObject, (2, false, "destroy-spaceport"));
REGISTER_OBJECT("old-school-destructable-object-3", OldSchoolDestructableObject, (3, false));
