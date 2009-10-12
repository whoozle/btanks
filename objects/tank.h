#ifndef __BT_PLAYER_H__
#define __BT_PLAYER_H__

/* Battle Tanks Game
 * Copyright (C) 2006-2009 Battle Tanks team
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

#include <string>
#include "object.h"
#include "alarm.h"

class Tank : public Object {
public:
	Tank(const std::string &classname);
	virtual Object * clone() const;
	virtual void on_spawn();

	virtual void emit(const std::string &event, Object * emitter);
	virtual void calculate(const float dt);
	virtual void tick(const float dt);
	virtual const bool take(const BaseObject *obj, const std::string &type);

	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);
	
	virtual const std::string getType() const { return "tank"; }

	void get_impassability_penalty(const float impassability, float &base, float &base_value, float &penalty) const;
	
private:
	Alarm _fire;
};

#endif

