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

class Machinegunner : public Object {
public:
	Machinegunner(const std::string &classname) : Object(classname) { impassability = 0; }
	virtual Object * clone() const { return new Machinegunner(*this); }
	virtual void onSpawn();
	virtual void tick(const float dt);
	virtual void calculate(const float dt);
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);
	virtual const bool take(const BaseObject *obj, const std::string &type);
};

void Machinegunner::onSpawn() {
	play("main", true);
}

void Machinegunner::tick(const float dt) {

}
void Machinegunner::calculate(const float dt) {

}

void Machinegunner::emit(const std::string &event, BaseObject * emitter) {}
const bool Machinegunner::take(const BaseObject *obj, const std::string &type) {
	return false;
}

REGISTER_OBJECT("machinegunner-on-launcher", Machinegunner, ("machinegunner-on-launcher"));
