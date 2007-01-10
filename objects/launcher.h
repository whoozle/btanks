#ifndef __BTANKS_LAUNCHER_H__
#define __BTANKS_LAUNCHER_H__

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

#include <string>
#include "object.h"
#include "alarm.h"

class Launcher : public Object {
public:
	Launcher();
	virtual Object * clone() const;

	virtual void emit(const std::string &event, Object * emitter);
	virtual const bool take(const BaseObject *obj, const std::string &type);
	virtual void tick(const float dt);
	virtual void calculate(const float dt);
	virtual void onSpawn();
	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);
	
private:
	Alarm _fire;
};

#endif

