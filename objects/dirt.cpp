
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
#include "config.h"

class Dirt : public Object {
public:
	Dirt() : Object("dirt") { pierceable = true; hp = -1; }
	virtual Object * clone() const;
	virtual void on_spawn();
	virtual void emit(const std::string &event, Object * emitter = NULL);
};

void Dirt::on_spawn() {
	if (registered_name.substr(0, 7) != "static-")
		play("fade-in", false);
	play("main", true);
	disown();
}

void Dirt::emit(const std::string &event, Object * emitter) {
	if (emitter != NULL && emitter->speed != 0 && event == "collision") {
		GET_CONFIG_VALUE("engine.drifting-duration", float, dd, 0.1);
		if (!emitter->has_effect("drifting"))
			emitter->add_effect("drifting", dd);
	} else Object::emit(event, emitter);
}


Object* Dirt::clone() const  {
	Object *a = new Dirt(*this);
	return a;
}

REGISTER_OBJECT("dirt", Dirt, ());
REGISTER_OBJECT("static-dirt", Dirt, ());
