
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
#include "resource_manager.h"
#include "game.h"

#include <set>

class Explosion : public Object {
public:
	Explosion(const std::string &classname) : Object(classname), _damaged_objects() { impassability = 0; }
	virtual void tick(const float dt);
	virtual Object * clone() const;
	virtual void onSpawn();
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);
	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		int n = _damaged_objects.size();
		s.add(n);
		for(std::set<int>::const_iterator i = _damaged_objects.begin(); i != _damaged_objects.end(); ++i) 
			s.add(*i);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		int n;
		s.get(n);
		_damaged_objects.clear();
		while(n--) {
			int id;
			s.get(id);
			_damaged_objects.insert(id);
		}
	}
private:
	std::set<int> _damaged_objects;
};


void Explosion::tick(const float dt) {
	Object::tick(dt);
	if (getState().empty()) {	
		//LOG_DEBUG(("over"));
		emit("death", this);
	}
}

void Explosion::onSpawn() {
	setDirection(0);
	if (classname == "smoke-cloud") {
		play("start", false);
		play("main", true);
	}
	
	play("boom", false);
	if (classname == "nuclear-explosion") 
		Game->shake(1, 4);
}

void Explosion::emit(const std::string &event, BaseObject * emitter) {
	if (event == "collision") {
		if (emitter->pierceable)
			return;
		
		if (classname == "nuclear-explosion") {
			//nuke damage.
			const int id = emitter->getID();
		
			if (_damaged_objects.find(id) != _damaged_objects.end())
				return; //damage was already added for this object.
		
			_damaged_objects.insert(id);
		
			emitter->addDamage(this, max_hp);
		} else if (classname == "smoke-cloud") {
			//poison cloud ;)
			
			const std::string &ec = emitter->classname;
			if (ec != "trooper" && ec != "citizen" && ec != "kamikaze")
				return;
			
			const int id = emitter->getID();
			if (_damaged_objects.find(id) != _damaged_objects.end())
				return; //damage was already added for this object.
			
			_damaged_objects.insert(id);
			emitter->addDamage(this, max_hp);
		}
		
	} else Object::emit(event, emitter);
}


Object* Explosion::clone() const  {
	return new Explosion(*this);
}

REGISTER_OBJECT("explosion", Explosion, ("explosion"));
REGISTER_OBJECT("nuclear-explosion", Explosion, ("nuclear-explosion"));
REGISTER_OBJECT("smoke-cloud", Explosion, ("smoke-cloud"));
