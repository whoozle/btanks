
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
#include "i18n.h"
#include "game_monitor.h"
#include "sound/mixer.h"
#include "mrt/random.h"
#include "player_manager.h"
#include "world.h"

#include <set>

class Explosion : public Object {
public:
	Explosion() : Object("explosion"), _damaged_objects(), _players_killed(0), _damage_done(false) { impassability = 0; }
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
		s.add(_players_killed);
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
		s.get(_players_killed);
	}

	void damageMap() const;
	
private:
	std::set<int> _damaged_objects;
	int _players_killed;
	bool _damage_done;
};

void Explosion::damageMap() const {
	//add damage for the map.
	v2<float> pos;
	getCenterPosition(pos);
	Map->damage(pos, max_hp, (size.x + size.y) / 4);
}


void Explosion::tick(const float dt) {
	Object::tick(dt);
	const std::string state = getState();

	GET_CONFIG_VALUE("objects.nuke-explosion.damage-map-after", float, dma, 0.65);

	if  (
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
	if (_variants.has("building")) {
		playRandomSound("building-explosion", false);
	}
	if (registered_name == "nuke-explosion" && !_variants.has("no-shaking")) 
		Game->shake(1, 4);
	disown();
}

void Explosion::emit(const std::string &event, Object * emitter) {
	if (event == "collision") {
		if (emitter == NULL || emitter->pierceable)
			return;
		
		if (emitter == NULL || registered_name == "explosion" || 
			(emitter->registered_name.size() >= 9 && 
			emitter->registered_name.substr(emitter->registered_name.size() - 9, 9) == "explosion")
			)
			return;
			
		const int id = emitter->getID();
		assert(emitter != NULL);
		
		if (_damaged_objects.find(id) != _damaged_objects.end())
			return; //damage was already added for this object.
		
		if (registered_name == "mutagen-explosion") {
			static std::set<std::string> mutable_classes;
			if (mutable_classes.empty()) {
				mutable_classes.insert("trooper");
				mutable_classes.insert("creature");
				mutable_classes.insert("kamikaze");
				mutable_classes.insert("civilian");
			}
			if (emitter->registered_name.compare(0, 6, "zombie") == 0) {
				emitter->hp = emitter->max_hp;
			}
			
			if (!PlayerManager->isClient() && mutable_classes.find(emitter->classname) != mutable_classes.end()) {
				//mutation 
				GET_CONFIG_VALUE("objects.mutagen-explosion.mutation-probability", float, mp, 0.3f);
				int p = mrt::random(1000);
				if (p < 1000 * mp) {
					//mutation
					std::string an = "mutated-" + emitter->registered_name;
					//LOG_DEBUG(("checking for animation '%s'", an.c_str()));
					if (ResourceManager->hasAnimation(an)) {
						emitter->init(an);
					} else {
						World->spawn(emitter, "zombie", "zombie", v2<float>(), v2<float>());
						emitter->Object::emit("death", this);
					}
				}
			}
		} else {
			emitter->addDamage(this, max_hp);
		}
		
		_damaged_objects.insert(id);
		
		if (emitter->isDead() && emitter->classname == "player") {
			++_players_killed;
			if (_players_killed == 2) {
				Mixer->playRandomSample(NULL, "laugh", false);
			}
		}
		need_sync = true;
	} else if (event == "death") {
		if (_players_killed > 1) {
			std::string combo = I18n->get("messages", "combo");
			GameMonitor->pushState(mrt::formatString("%dx %s", _players_killed, combo.c_str()), 2);
		}
		Object::emit(event, emitter);
	} else Object::emit(event, emitter);
}


REGISTER_OBJECT("explosion", Explosion, ());
REGISTER_OBJECT("nuke-explosion", Explosion, ());
REGISTER_OBJECT("cannon-explosion", Explosion, ());
REGISTER_OBJECT("mortar-explosion", Explosion, ());
REGISTER_OBJECT("mutagen-explosion", Explosion, ());
