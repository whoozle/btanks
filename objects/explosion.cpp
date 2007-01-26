
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
#include "tmx/map.h"
#include "config.h"
#include "game.h"

#include <set>

class Explosion : public Object {
public:
	Explosion() : Object("explosion"), _damaged_objects(), _damage_done(false) { impassability = 0; }
	Object* clone() const  { return new Explosion(*this); }

	virtual void tick(const float dt);
	virtual void onSpawn();
	virtual void emit(const std::string &event, Object * emitter = NULL);
	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		int n = _damaged_objects.size();
		s.add(n);
		for(std::set<int>::const_iterator i = _damaged_objects.begin(); i != _damaged_objects.end(); ++i) 
			s.add(*i);
		s.add(_damage_done);
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
		s.get(_damage_done);
	}

	void damageMap() const;
	
private:
	std::set<int> _damaged_objects;
	bool _damage_done;
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
		for(p.x = position.x; p.x < position2.x; p.x += tile_size.x) {
			
			if ((p-center).quick_length() <= r) {
				//LOG_DEBUG(("skipped %g %g", p.x, p.y));
					Map->damage(p, max_hp);
			}
				//LOG_DEBUG(("%g %g", p.x, p.y));
		}
	}
}


void Explosion::tick(const float dt) {
	Object::tick(dt);
	const std::string state = getState();

	GET_CONFIG_VALUE("objects.nuclear-explosion.damage-map-after", float, dma, 0.65);

	if  (
			(registered_name == "nuclear-explosion" || registered_name == "cannon-explosion") && 
			!_damage_done && getStateProgress() >= dma && state != "start"
		) {
		_damage_done = true;
		damageMap();
	}

	if (state.empty()) {	
		emit("death", this);
	}
}

void Explosion::onSpawn() {
	play("boom", false);
	if (registered_name == "nuclear-explosion") 
		Game->shake(1, 4);
}

void Explosion::emit(const std::string &event, Object * emitter) {
	if (event == "collision") {
		if (emitter == NULL || emitter->pierceable)
			return;
		
		if (registered_name == "nuclear-explosion" || registered_name == "cannon-explosion") {
			//nuke damage.
			if (emitter == NULL || 
				emitter->registered_name == "nuclear-explosion" || 
				emitter->registered_name == "cannon-explosion")
				return;
			
			const int id = emitter->getID();
		
			if (_damaged_objects.find(id) != _damaged_objects.end())
				return; //damage was already added for this object.
		
			_damaged_objects.insert(id);
		
			emitter->addDamage(this, max_hp);
		} 		
	} else Object::emit(event, emitter);
}


REGISTER_OBJECT("explosion", Explosion, ());
REGISTER_OBJECT("nuclear-explosion", Explosion, ());
REGISTER_OBJECT("cannon-explosion", Explosion, ());
