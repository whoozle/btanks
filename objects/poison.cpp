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

class PoisonCloud : public Object {
public:
	PoisonCloud() : Object("poison"), _damage(true) { pierceable = true; }
	virtual Object * clone() const { return new PoisonCloud(*this); }
	virtual void onSpawn();
	virtual void tick(const float dt);
	virtual void emit(const std::string &event, Object * emitter = NULL);

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add((int)_damaged_objects.size());
		for(std::set<int>::const_iterator i = _damaged_objects.begin(); i != _damaged_objects.end(); ++i) 
			s.add(*i);
		s.add(_damage);
	}

	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		int n; s.get(n);
		_damaged_objects.clear();
		while(n--) {
			int id;
			s.get(id);
			_damaged_objects.insert(id);
		}
		s.get(_damage);
	}

private: 
	std::set<int> _damaged_objects;
	Alarm _damage;
};

void PoisonCloud::tick(const float dt) {
	Object::tick(dt);
	if (_damage.tick(dt)) {
		_damaged_objects.clear();
		need_sync = true;
	}
}

void PoisonCloud::onSpawn() {
	float di;
	Config->get("objects." + registered_name + ".damage-interval", di, 1);
	_damage.set(di);
	
	if (registered_name.substr(0, 7) != "static-")
		play("start", false);
	
	play("main", true);
	disown();
}

void PoisonCloud::emit(const std::string &event, Object * emitter) {
	if (event == "collision") {
		if (emitter == NULL)
			return;
		
		const std::string &ec = emitter->classname;
		if (ec != "trooper" && ec != "citizen" && 
			ec != "kamikaze" && ec != "watchtower" && 
			ec != "monster" && 
			emitter->registered_name != "machinegunner-player")
			return;
		
		const int id = emitter->getID();
		if (_damaged_objects.find(id) != _damaged_objects.end())
			return; //damage was already added for this object.
		
		_damaged_objects.insert(id);
		emitter->addDamage(this, max_hp);		
	} else Object::emit(event, emitter);
}

REGISTER_OBJECT("smoke-cloud", PoisonCloud, ());
REGISTER_OBJECT("static-smoke-cloud", PoisonCloud, ());
