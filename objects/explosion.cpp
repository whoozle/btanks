
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
#include "tmx/map.h"

#include <set>

class Explosion : public Object {
public:
	Explosion(const std::string &classname) : Object(classname), _damaged_objects() { impassability = 0; }
	virtual void tick(const float dt);
	virtual Object * clone() const;
	virtual void onSpawn();
	virtual void emit(const std::string &event, Object * emitter = NULL);
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

	void damageMap() const;
	
private:
	std::set<int> _damaged_objects;
};

void Explosion::damageMap() const {
	//add damage for the map.
	v3<int> tile_size = Map->getTileSize();
	
	v3<float> position;
	getPosition(position);
	v3<float> position2 = position + size, center = position + size/2;
	
	v3<float> p;
	float r = (size.x + size.y) / 4;
	r *= r;
	for(p.y = position.y; p.y < position2.y; p.y += tile_size.y) {
		for(p.x = position.y; p.x < position2.x; p.x += tile_size.x) {
			if ((p-center).quick_length() > r) {
				//LOG_DEBUG(("skipped %g %g", p.x, p.y));
				continue;
			}
			//LOG_DEBUG(("%g %g", p.x, p.y));
			Map->damage(p, max_hp);
		}
	}
}


void Explosion::tick(const float dt) {
	Object::tick(dt);
	if (getState().empty()) {	
		//LOG_DEBUG(("over"));
		if (classname == "nuclear-explosion") {
			damageMap();
		}
		emit("death", this);
	}
}

void Explosion::onSpawn() {
	if (classname == "smoke-cloud") {
		play("start", false);
		play("main", true);
		return;
	}
	
	play("boom", false);
	if (classname == "nuclear-explosion") 
		Game->shake(1, 4);
}

void Explosion::emit(const std::string &event, Object * emitter) {
	if (event == "collision") {
		if (emitter == NULL || emitter->pierceable)
			return;
		
		if (classname == "nuclear-explosion") {
			//nuke damage.
			if (emitter == NULL || emitter->classname == "nuclear-explosion")
				return;
			
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
