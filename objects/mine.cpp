
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

class Mine : public Object {
public:
	Mine() : Object("mine") { piercing = false; pierceable = true; }
	virtual Object * clone() const;
	virtual void onSpawn();
	virtual void tick(const float dt);
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);
};

void Mine::onSpawn() {
	play("3", false);
	play("pause", false);
	play("2", false);
	play("pause", false);
	play("1", false);
	play("pause", false);
	play("armed", true);
}

void Mine::tick(const float dt) {
	Object::tick(dt);
	if (getState() == "armed") 
		disown();
}

void Mine::emit(const std::string &event, BaseObject * emitter) {
	if (event == "collision") {
		if (getState() == "armed") {
			spawn("explosion", "explosion");
			Object::emit("death", emitter);
			emitter->addDamage(this, hp);
		} 
	} else Object::emit(event, emitter);
}


Object* Mine::clone() const  {
	return new Mine(*this);
}

REGISTER_OBJECT("regular-mine", Mine, ());
