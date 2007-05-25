#ifndef _BTANKS_ITEM_OBJECT_H__
#define _BTANKS_ITEM_OBJECT_H__

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

class Item : public Object {
public:
	std::string type;
	Item(const std::string &classname, const std::string &type = std::string());
	virtual Object * clone() const;
	virtual void onSpawn();
	virtual void tick(const float dt);
	virtual void emit(const std::string &event, Object * emitter = NULL);
	virtual void addDamage(Object *from, const int hp, const bool emitDeath = true);

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(type);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(type);
	}
	virtual const std::string getType() const { return type; }
	virtual const int getCount() const { return hp; }

};

#endif
