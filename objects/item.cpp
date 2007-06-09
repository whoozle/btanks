
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

#include "item.h"
#include "resource_manager.h"

Item::Item(const std::string &classname, const std::string &type) : Object(classname), type(type) {
	pierceable = true;
	impassability = 1;
}


void Item::tick(const float dt) {
	Object::tick(dt);
	if (getState().empty()) 
		Object::emit("death", this);
}

void Item::onSpawn() {
	play("main", true);
}

void Item::addDamage(Object *from, const int hp, const bool emitDeath) {
	return;
}

void Item::emit(const std::string &event, Object * emitter) {
	if (event == "collision") {
		if (emitter == NULL || emitter->classname != "player")
			return;
		
		if (!emitter->take(this, type)) {
			return;
		}

		hp = 0;
		impassability = 0;
		setZ(999); //fly up on the vehicle
		cancelAll();
		play("take", false);
	} else Object::emit(event, emitter);
}


Object* Item::clone() const  {
	return new Item(*this);
}

/*  note that all heal objects have the same classname. this was done to simplify AI search/logic.*/

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
