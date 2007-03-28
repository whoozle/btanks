
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
#include "player_slot.h"
#include "world.h"
#include "controls/control_method.h"
#include "tooltip.h"

PlayerSlot::PlayerSlot() : 
id(-1), control_method(NULL), need_sync(false), dont_interpolate(false), remote(false), trip_time(10), visible(false), 
classname(), animation(), frags(0), reserved(false)
{}

PlayerSlot::PlayerSlot(const int id) : 
id(id), control_method(NULL), need_sync(false), dont_interpolate(false), remote(false), trip_time(10), visible(false), 
classname(), animation(), frags(0), reserved(false)
{}

void PlayerSlot::serialize(mrt::Serializator &s) const {
	s.add(id);
	//ControlMethod * control_method;
	position.serialize(s);
	s.add(frags);		
	s.add(classname);
	s.add(animation);
}

void PlayerSlot::deserialize(const mrt::Serializator &s) {
	s.get(id);
	position.deserialize(s);
	s.get(frags);		
	s.get(classname);
	s.get(animation);
}


Object * PlayerSlot::getObject() {
	if (id < 0) 
		return NULL;
	Object *o = World->getObjectByID(id);
	return o;
}
const Object * PlayerSlot::getObject() const {
	if (id < 0) 
		return NULL;
	const Object *o = World->getObjectByID(id);
	return o;
}


void PlayerSlot::clear() {
	id = -1;
	if (control_method != NULL) {
		delete control_method; 
		control_method = NULL;
	}
	animation.clear();
	classname.clear();

	need_sync = false;
	remote = false;
	frags = 0;
	reserved = false;
	
	zones_reached.clear();
	while(!tooltips.empty()) {
		delete tooltips.front().second;
		tooltips.pop();
	}
}

PlayerSlot::~PlayerSlot() {
	clear();
}
