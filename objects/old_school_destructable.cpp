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

#include "object.h"
#include "registrar.h"
#include "object.h"

class OldSchoolDestructableObject : public Object {
public:
	OldSchoolDestructableObject(const int hops);

	virtual Object * clone() const { return new OldSchoolDestructableObject(*this); }
	virtual void tick(const float dt);
	virtual void on_spawn();
	virtual void add_damage(Object *from, const int hp, const bool emitDeath = true);

	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);

protected:
	int _hops;
	int _explosions;

private: 
	Alarm _spawn;
};

#include "mrt/random.h"
#include "config.h"


OldSchoolDestructableObject::OldSchoolDestructableObject(const int hops) : 
		Object("destructable-object"), 
		_hops(hops),
		_explosions(0), 
		_spawn(true) {}

void OldSchoolDestructableObject::serialize(mrt::Serializator &s) const {
	Object::serialize(s);
	s.add(_hops);
	s.add(_explosions);
	s.add(_spawn);
}

void OldSchoolDestructableObject::deserialize(const mrt::Serializator &s) {
	Object::deserialize(s);
	s.get(_hops);
	s.get(_explosions);
	s.get(_spawn);
}

void OldSchoolDestructableObject::add_damage(Object *from, const int dhp, const bool emitDeath) {
	if (_hops <= 0)
		return;

	Object::add_damage(from, dhp, false);
	if (hp <= 0 && _explosions == 0) {
		Config->get("objects." + registered_name + ".explosions", _explosions, 16);		
		hp = -1;
	}
}

void OldSchoolDestructableObject::tick(const float dt) {
	Object::tick(dt);
	
	if (!_spawn.tick(dt))
		return;
	
	if (_explosions != 0) {
		int e;
		Config->get("objects." + registered_name + ".explosions", e, 16);		
		
		if (_explosions == (e + 1)/2) {
			--_hops;

			cancel_all();

			if (_hops == 0) {
				//completely dead
				hp = -1; 
				play("broken", true);
			} else {
				hp = max_hp;
				play(mrt::format_string("damaged-%d", _hops), true);
			}
		}
		
		v2<float> dpos; 
		dpos.x = mrt::random((int)size.x) - size.x / 2;
		dpos.y = mrt::random((int)size.y) - size.y / 2;
		
		spawn("explosion", "building-explosion", dpos);
		--_explosions;
	}
}

void OldSchoolDestructableObject::on_spawn() {
	_spawn.set(0.2f);
	play("main", true);
}

REGISTER_OBJECT("old-school-destructable-object-2", OldSchoolDestructableObject, (2));
REGISTER_OBJECT("spaceport-baykonur", OldSchoolDestructableObject, (2));
REGISTER_OBJECT("old-school-destructable-object-3", OldSchoolDestructableObject, (3));
