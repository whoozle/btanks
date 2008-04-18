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

class CTFBase : public Object {
public:
	CTFBase() : Object("ctf-base") {
		impassability = -1;
		hp = -1;
	}
	virtual Object * clone() const { return CTFBase(*this); }
	
	virtual void tick(const float dt);

	void onSpawn() {
		play("main", true);
	}

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
	}

	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
	}

private:
};

void SinglePose::tick(const float dt) {
	Object::tick(dt);
}


REGISTER_OBJECT("ctf-base", CTFBase, ());
