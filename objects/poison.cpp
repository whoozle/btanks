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
#include "config.h"

class PoisonCloud : public Object {
public:
	PoisonCloud() : Object("poison"), _damage(true) { pierceable = true; }
	virtual Object * clone() const { return new PoisonCloud(*this); }
	virtual void on_spawn();
	virtual void tick(const float dt);
	virtual void emit(const std::string &event, Object * emitter = NULL);

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_damaged_objects);
		s.add(_damage);
	}

	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_damaged_objects);
		s.get(_damage);
	}

private: 
	std::set<int> _damaged_objects;
	Alarm _damage;
};

void PoisonCloud::tick(const float dt) {
	Object::tick(dt);
	if (_damage.tick(dt)) {
		_damaged_objects.clear();
	}
}

void PoisonCloud::on_spawn() {
	float di;
	Config->get("objects." + registered_name + ".damage-interval", di, 1);
	_damage.set(di);
	
	if (registered_name.substr(0, 7) != "static-")
		play("start", false);
	
	play("main", true);
	disown();
}

void PoisonCloud::emit(const std::string &event, Object * emitter) {
	if (event == "collision") {
		if (emitter == NULL)
			return;
		
		const std::string &ec = emitter->classname;
		if (ec != "trooper" && ec != "civilian" && 
			ec != "kamikaze" && ec != "watchtower" && 
			ec != "monster" && ec != "cannon" && 
			emitter->registered_name != "machinegunner")
			return;
		
		const int id = emitter->get_id();
		if (_damaged_objects.find(id) != _damaged_objects.end())
			return; //damage was already added for this object.
		
		_damaged_objects.insert(id);
		if (emitter->get_variants().has("poison-resistant")) {
			return;
		}
		emitter->add_damage(this, max_hp);
	} else Object::emit(event, emitter);
}

REGISTER_OBJECT("smoke-cloud", PoisonCloud, ());
REGISTER_OBJECT("static-smoke-cloud", PoisonCloud, ());
