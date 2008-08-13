
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

#include "item.h"
#include "registrar.h"

Item::Item(const std::string &classname, const std::string &type) : Object(classname), type(type) {
	pierceable = true;
	impassability = -1;
	speed = 0;
	set_directions_number(1);
}


void Item::tick(const float dt) {
	Object::tick(dt);
	if (get_state().empty()) 
		Object::emit("death", this);
}

void Item::on_spawn() {
	play("main", true);
}

void Item::add_damage(Object *from, const int hp, const bool emitDeath) {
	return;
}

void Item::emit(const std::string &event, Object * emitter) {
	if (event == "collision") {
		if (emitter == NULL)
			return;
		
		if (!emitter->take(this, type)) {
			return;
		}

		hp = 0;
		impassability = 0;
		set_z(999); //fly up on the vehicle
		cancel_all();
		play("take", false);
	} else Object::emit(event, emitter);
}


Object* Item::clone() const  {
	return new Item(*this);
}

REGISTER_OBJECT("base-item", Item, ("dummy"));

REGISTER_OBJECT("heal", Item, ("heal"));
REGISTER_OBJECT("megaheal", Item, ("heal"));

REGISTER_OBJECT("guided-missiles-item", Item, ("missiles", "guided"));
REGISTER_OBJECT("dumb-missiles-item", Item, ("missiles", "dumb"));
REGISTER_OBJECT("smoke-missiles-item", Item, ("missiles", "smoke"));
REGISTER_OBJECT("nuke-missiles-item", Item, ("missiles", "nuke"));
REGISTER_OBJECT("boomerang-missiles-item", Item, ("missiles", "boomerang"));
REGISTER_OBJECT("stun-missiles-item", Item, ("missiles", "stun"));
REGISTER_OBJECT("mutagen-missiles-item", Item, ("missiles", "mutagen"));

REGISTER_OBJECT("mines-item", Item, ("mines", "regular"));

REGISTER_OBJECT("dirt-bullets-item", Item, ("effects", "dirt"));
REGISTER_OBJECT("dispersion-bullets-item", Item, ("effects", "dispersion"));
REGISTER_OBJECT("ricochet-bullets-item", Item, ("effects", "ricochet"));
REGISTER_OBJECT("machinegunner-item", Item, ("mod", "machinegunner"));
REGISTER_OBJECT("thrower-item", Item, ("mod", "thrower"));

REGISTER_OBJECT("invulnerability-item", Item, ("effects", "invulnerability"));
REGISTER_OBJECT("speedup-item", Item, ("effects", "speedup"));
REGISTER_OBJECT("slowdown-item", Item, ("effects", "slowdown"));
