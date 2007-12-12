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
#include "registrar.h"
#include "ai/herd.h"
#include "ai/trooper.h"
#include "config.h"

class Slime : public Object, ai::StupidTrooper, ai::Herd {
public: 
	Slime() : Object("monster"), ai::StupidTrooper("bullet", false) {}
	Object *clone() const { return new Slime(*this); }

	const int getComfortDistance(const Object *other) const {
		GET_CONFIG_VALUE("objects.slime.comfort-distance", int, cd, 120);
		return (other == NULL || other->registered_name == registered_name)?cd:-1; //fixme names if you want
	}
	
	void onIdle() {
		_state.fire = false;

		float tt = getWeaponRange("bullet");
		ai::Herd::calculateV(_velocity, this, 0, tt);
	}
	
	void onSpawn() {
		ai::StupidTrooper::onSpawn();
	}
};

REGISTER_OBJECT("slime", Slime, ());
