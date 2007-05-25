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

#include "heli.h"
#include "resource_manager.h"
#include "alarm.h"
#include "config.h"

Heli::Heli(const std::string &classname) : 
	Object(classname), _fire(false), _alt_fire(false), _left(false) {
	impassability = -1;	
}


void Heli::calculate(const float dt) {
	Object::calculate(dt);

	GET_CONFIG_VALUE("objects.helicopter.rotation-time", float, rt, 0.2);
	limitRotation(dt, rt, true, true);	
}


void Heli::tick(const float dt) {
	Object::tick(dt);
	
	if (_state.fire && _fire.tick(dt)) {
		_fire.reset();
		spawn("helicopter-bullet", _left?"helicopter-bullet-left":"helicopter-bullet-right", v2<float>::empty, _direction);
		_left = !_left;
	}
	if (_state.alt_fire && _alt_fire.tick(dt)) {
		_alt_fire.reset();
		Object *o = spawn("bomb", "bomb");
		o->setZ(getZ() - 1, true);
	}
}

void Heli::onSpawn() {
	GET_CONFIG_VALUE("objects.helicopter.fire-rate", float, fr, 0.1);
	_fire.set(fr);

	GET_CONFIG_VALUE("objects.helicopter.bombing-rate", float, br, 0.2);
	_alt_fire.set(br);

	play("move", true);
}

void Heli::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
		spawn("explosion", "nuclear-explosion");
	} else if (event == "collision") {
	}
	
	Object::emit(event, emitter);
}


Object* Heli::clone() const  {
	return new Heli(*this);
}

REGISTER_OBJECT("helicopter-player", Heli, ("player"));
