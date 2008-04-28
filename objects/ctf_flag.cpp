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

#include "object.h"
#include "registrar.h"
#include "team.h"
#include "player_manager.h"
#include "world.h"

class CTFFlag : public Object {
public:
	void tick(const float dt) {
		Object::tick(dt);
	}
	
	void emit(const std::string &event, Object * emitter) {
		if (event == "collision") {
			//add flag handling here.
			if (emitter == NULL || !emitter->getVariants().has("player")) {
				return;
			}
			
			Team::ID team = Team::get_team(this);
			assert(team != Team::None);
				
			PlayerSlot *slot = PlayerManager->getSlotByID(emitter->getID());
			if (slot == NULL) {
				return;
			}

			if (slot->team == team) {
				//LOG_DEBUG(("returning to base"));

				//return to the base
				int base_id = getSummoner(); //FIX THIS. summoner could be dead player
				const Object *base = World->getObjectByID(base_id);
				if (base != NULL) {
					v2<float> dpos = getRelativePosition(base);
					if (dpos.quick_length() > size.x * size.y / 4)
						World->teleport(this, base->getCenterPosition());
				} else {
					LOG_WARN(("could not find base %d", base_id));
				}
				//check color and team ! 
				if (emitter->has("#ctf-flag")) {
					LOG_DEBUG(("frag! frag!"));
					emitter->remove("#ctf-flag");
					return;
				}
			}

			
			emitter->add("#ctf-flag", "ctf-flag-on-vehicle", animation, v2<float>(), Centered);
			emit("death", this);
		} else Object::emit(event, emitter);
	}

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
	}

	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
	}

	CTFFlag() : Object("ctf-flag") {
		impassability = -1;
		hp = -1;
		setDirectionsNumber(1);
	}
	
	virtual Object * clone() const { return new CTFFlag(*this); }
	
	void onSpawn() {
		play("main", true);
	}

private:
};

REGISTER_OBJECT("ctf-flag", CTFFlag, ());
