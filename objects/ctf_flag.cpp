/* Battle Tanks Game
 * Copyright (C) 2006-2009 Battle Tanks team
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
			if (emitter == NULL || !emitter->get_variants().has("player")) {
				return;
			}
			
			Team::ID team = Team::get_team(this);
			assert(team != Team::None);
				
			PlayerSlot *slot = PlayerManager->get_slot_by_id(emitter->get_id());
			if (slot == NULL) {
				return;
			}

			int base_id = get_summoner(); 
			Object *base = World->getObjectByID(base_id);
			if (slot->team == team) {
				if (base != NULL) {
					v2<float> dpos = get_relative_position(base);
					if (dpos.quick_length() > size.x * size.y / 4) {
						set_zbox(base->get_z());
						World->teleport(this, base->get_center_position());
						base->remove_effect("abandoned");
					} else {
						//my flag is close to the base. if i have foreign flag, frag it
						if (emitter->has("#ctf-flag")) {
							Object *flag = emitter->drop("#ctf-flag");
							++slot->frags;
							PlayerManager->action(*slot, "ctf");
							Object *base = World->getObjectByID(flag->get_summoner());
							if (base != NULL) {
								set_zbox(base->get_z());
								World->teleport(flag, base->get_center_position());
								base->remove_effect("abandoned");
							} else {
								LOG_WARN(("could not find base for the flag %s", flag->animation.c_str()));
							}
						}
					}
				} else {
					LOG_WARN(("could not find base %d", base_id));
				}
			} else {
				if (base != NULL)
					base->add_effect("abandoned", -1);
				if (!emitter->has("#ctf-flag"))  //just for the future expansion
					emitter->pick("#ctf-flag", this);
			}
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
		set_directions_number(1);
		pierceable = true;
	}
	
	virtual Object * clone() const { return new CTFFlag(*this); }
	
	void on_spawn() {
		play("main", true);
	}

private:
};

REGISTER_OBJECT("ctf-flag", CTFFlag, ());
