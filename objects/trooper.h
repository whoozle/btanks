#ifndef BTANKS_TROOPER_H_
#define BTANKS_TROOPER_H_

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


#include "object.h"
#include "alarm.h"

class Trooper : public Object {
public:
	Trooper(const std::string &classname, const std::string &object) : 
		Object(classname), _object(object), _fire(false), _alt_fire(false) {}
	
	virtual void tick(const float dt);

	virtual Object * clone() const;
	virtual void on_spawn();
	virtual void emit(const std::string &event, Object * emitter = NULL);
	virtual const bool validateFire(const int idx);

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_object);
		s.add(_fire);
		s.add(_alt_fire);
		s.add(_pose);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_object);
		s.get(_fire);
		s.get(_alt_fire);
		s.get(_pose);
	}	

	void get_impassability_penalty(const float impassability, float &base, float &base_value, float &penalty) const;
	virtual const bool take(const BaseObject *obj, const std::string &type);

protected: 
	std::string _object;
	Alarm _fire, _alt_fire;
	
	std::string _pose; //run by default
};

#endif
