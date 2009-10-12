
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
#include "registrar.h"
#include "object.h"
#include "config.h"
#include "mrt/random.h"
#include "sdlx/surface.h"

class TooltipObject : public Object {
public:
	TooltipObject() : Object("tooltip"), _change(true) { impassability = 0; hp = -1; }
	virtual void on_spawn() {
		GET_CONFIG_VALUE("objects.random-tooltip.show-time", float, st, 3.0);
		_change.set(st);
	
		const sdlx::Surface * s = get_surface();
		int w = s->get_width();
		int n = (w - 1) / (int)size.x + 1;
		set_directions_number(n);
		//LOG_DEBUG(("dirs = %d", n));
		Object::set_direction(mrt::random(n));
		play("main", true);
	}
	
	virtual void tick(const float dt) {
		Object::tick(dt);
		if (_change.tick(dt)) {
			Object::set_direction(mrt::random(get_directions_number()));
		}
	}
	
	virtual void set_direction(const int dir) {}

	virtual Object * clone() const {
		return new TooltipObject(*this);
	}

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_change);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_change);	
	}

private: 
	Alarm _change;
};

REGISTER_OBJECT("random-tooltip", TooltipObject, ());
