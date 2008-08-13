
/* Battle Tanks Game
 * Copyright (C) 2006-2008 Battle Tanks team
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

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
*/

#include "object.h"
#include "registrar.h"
#include "tmx/map.h"
#include "config.h"
#include "game.h"
#include "i18n.h"
#include "game_monitor.h"
#include "sound/mixer.h"
#include "mrt/random.h"
#include "resource_manager.h"

#include <set>

class Explosion : public Object {
public:
	Explosion() : Object("explosion"), _damaged_objects(), _players_killed(0), _damage_done(false) { hp = -1; impassability = 0; pierceable = true; }
	Object* clone() const  { return new Explosion(*this); }

	virtual void tick(const float dt);
	virtual void on_spawn();
	virtual void emit(const std::string &event, Object * emitter = NULL);
	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_damaged_objects);
		s.add(_damage_done);
		s.add(_players_killed);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_damaged_objects);
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
	get_center_position(pos);
	Map->damage(pos, max_hp, (size.x + size.y) / 4);
}


void Explosion::tick(const float dt) {
	Object::tick(dt);
	const std::string state = get_state();

	GET_CONFIG_VALUE("objects.nuke-explosion.damage-map-after", float, dma, 0.65);

	if  (
			!_damage_done && get_state_progress() >= dma && state != "start"
		) {
		_damage_done = true;
	
		if (registered_name != "mutagen-explosion")
			damageMap();
	}

	if (state.empty()) {	
		emit("death", this);
	}
}

void Explosion::on_spawn() {
	play("boom", false);
	if (_variants.has("building")) {
		play_random_sound("building-explosion", false);
	}
	if (registered_name == "nuke-explosion" && !_variants.has("no-shaking")) 
		Game->shake(1, 4);
	disown();
}

#include "player_manager.h"
#include "world.h"

void Explosion::emit(const std::string &event, Object * emitter) {
	if (event == "collision") {
		if (emitter == NULL || registered_name == "explosion" || 
			(emitter->registered_name.size() >= 9 && 
			emitter->registered_name.substr(emitter->registered_name.size() - 9, 9) == "explosion")
			|| emitter->classname == "poison"
			)
			return;
			
		const int id = emitter->get_id();
		assert(emitter != NULL);
		
		if (_damaged_objects.find(id) != _damaged_objects.end())
			return; //damage was already added for this object.
		
		if (registered_name == "mutagen-explosion") {
			if (_variants.has("chained") && emitter->classname == "explosive" && emitter->get_state() == "main") {
				float p = get_state_progress();
			//LOG_DEBUG(("%d: progress = %g", get_id(), p));
				if (p < 0.03f)
					return;
				emitter->emit("destroy", this);
				_damaged_objects.insert(id);
				return;
			}
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
			
			if (mutable_classes.find(emitter->classname) != mutable_classes.end()) {
				//mutation 
				GET_CONFIG_VALUE("objects.mutagen-explosion.mutation-probability", float, mp, 0.5f);
				int p = mrt::random(1000);
				if (_variants.has("100%") || p < 1000 * mp) {
					//mutation
					std::string an = "mutated-" + emitter->registered_name;
					//LOG_DEBUG(("checking for animation '%s'", an.c_str()));
					if (ResourceManager->hasAnimation(an)) {
						emitter->init(an);
					} else {
						Object * zombie = emitter->spawn((p&1)? "zombie": "slime", (p&1)? "zombie": "slime");
						emitter->attachVehicle(zombie);
					}
				}
			}
		} else {
			emitter->add_damage(this, max_hp);
		}
		
		if (!emitter->is_dead()) 
			_damaged_objects.insert(id);
		
		if (emitter->is_dead() && emitter->classname == "player") {
			++_players_killed;
			if (_players_killed == 2) {
				Mixer->playRandomSample(NULL, "laugh", false);
			}
		}
		invalidate();
	} else if (event == "death") {
		if (_players_killed > 1) {
			std::string combo = I18n->get("messages", "combo");
			GameMonitor->pushState(mrt::format_string("%dx %s", _players_killed, combo.c_str()), 2);
		}
		Object::emit(event, emitter);
	} else Object::emit(event, emitter);
}


REGISTER_OBJECT("explosion", Explosion, ());
REGISTER_OBJECT("nuke-explosion", Explosion, ());
REGISTER_OBJECT("cannon-explosion", Explosion, ());
REGISTER_OBJECT("mortar-explosion", Explosion, ());
REGISTER_OBJECT("grenade-explosion", Explosion, ());
REGISTER_OBJECT("mutagen-explosion", Explosion, ());
REGISTER_OBJECT("kamikaze-explosion", Explosion, ());
