#ifndef __BTANKS_MORTAR_H__
#define __BTANKS_MORTAR_H__

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

#include <string>
#include "object.h"
#include "alarm.h"

class Mortar : virtual public Object {
public:
	Mortar(const std::string &classname);
	virtual Object * clone() const;
	virtual void onSpawn();

	virtual void emit(const std::string &event, Object * emitter);
	virtual void calculate(const float dt);
	virtual void tick(const float dt);
	virtual const bool take(const BaseObject *obj, const std::string &type);

	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);
	
	virtual const std::string getType() const { return "mortar"; }

	void getImpassabilityPenalty(const float impassability, float &base, float &base_value, float &penalty) const;
	
private:
	Alarm _fire;
};

#endif

