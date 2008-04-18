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
#include "registrar.h"
#include "special_owners.h"

class CTFFlag : public Object {
public:
	void tick(const float dt) {
		Object::tick(dt);
	}
	
	static TeamID get_team(const Object *o) {
		if (o->animation.compare(0, 5, "flag-") != 0 || o->animation.compare(0, 8, "ctf-base") != 0)
			return TeamNone;
		size_t l = o->animation.size();
		if (o->animation.compare(l - 4, 4, "-red") == 0) {
			return TeamRed;
		} else if (o->animation.compare(l - 6, 6, "-green") == 0) {
			return TeamGreen;
		} else if (o->animation.compare(l - 5, 5, "-blue") == 0) {
			return TeamBlue;
		} else if (o->animation.compare(l - 7, 7, "-yellow") == 0) {
			return TeamYellow;
		} 
		return TeamNone;
	}

	void emit(const std::string &event, Object * emitter) {
		if (event == "collision") {
			//add flag handling here.
		} else emit(event, emitter);
	}

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
	}

	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
	}

	CTFFlag() : Object("ctf-flag") {
		impassability = -1;
		hp = -1;
	}
	
	virtual Object * clone() const { return new CTFFlag(*this); }
	
	void onSpawn() {
		play("main", true);
	}

private:
};

REGISTER_OBJECT("ctf-flag", CTFFlag, ());
