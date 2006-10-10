
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

class Dirt : public Object {
public:
	Dirt() : Object("dirt") { pierceable = true; }
	virtual Object * clone() const;
	virtual void onSpawn();
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);
};

void Dirt::onSpawn() {
	setDirection(0);
	play("fade-in", false);
	play("main", true);
}

void Dirt::emit(const std::string &event, BaseObject * emitter) {
	if (event == "collision") {
		return;
	} else Object::emit(event, emitter);
}


Object* Dirt::clone() const  {
	Object *a = new Dirt(*this);
	return a;
}

REGISTER_OBJECT("dirt", Dirt, ());
