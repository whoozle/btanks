
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
#include "config.h"

class Dirt : public Object {
public:
	Dirt() : Object("dirt") { pierceable = true; hp = -1; }
	virtual Object * clone() const;
	virtual void onSpawn();
	virtual void emit(const std::string &event, Object * emitter = NULL);
};

void Dirt::onSpawn() {
	if (registered_name.substr(0, 7) != "static-")
		play("fade-in", false);
	play("main", true);
	disown();
}

void Dirt::emit(const std::string &event, Object * emitter) {
	if (emitter != NULL && emitter->speed != 0 && event == "collision") {
		GET_CONFIG_VALUE("engine.drifting-duration", float, dd, 0.1);
		if (!emitter->isEffectActive("drifting"))
			emitter->addEffect("drifting", dd);
	} else Object::emit(event, emitter);
}


Object* Dirt::clone() const  {
	Object *a = new Dirt(*this);
	return a;
}

REGISTER_OBJECT("dirt", Dirt, ());
REGISTER_OBJECT("static-dirt", Dirt, ());
