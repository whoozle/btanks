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
#include "resource_manager.h"
#include "config.h"
#include "destructable_object.h"
#include "mrt/random.h"

//remove trooper from here. :)

class PillBox : public DestructableObject {
public: 
	PillBox(const std::string &object, const bool aim_missiles) : 
		DestructableObject("trooper"), _reaction(true), _fire(false), _object(object) {
			if (aim_missiles)
				_targets.insert("missile");
	
			_targets.insert("fighting-vehicle");
			_targets.insert("monster");
			_targets.insert("trooper");
			_targets.insert("kamikaze");
			_targets.insert("boat");		
			_targets.insert("helicopter");
	}

	virtual Object * clone() const { return new PillBox(*this); }
	
	virtual void onSpawn() { 
		GET_CONFIG_VALUE("objects.pillbox.reaction-time", float, rt, 0.1);
		mrt::randomize(rt, rt/10);
		_reaction.set(rt);

		GET_CONFIG_VALUE("objects.pillbox.fire-rate", float, fr, 0.2);
		_fire.set(fr);
		
		DestructableObject::onSpawn();
	}

	virtual void serialize(mrt::Serializator &s) const {
		DestructableObject::serialize(s);
		s.add(_reaction);
		s.add(_fire);
		s.add(_object);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		DestructableObject::deserialize(s);
		s.get(_reaction);
		s.get(_fire);
		s.get(_object);
	}
	
	virtual void tick(const float dt) {
		Object::tick(dt);
		if (!_broken && _fire.tick(dt) && _state.fire) {
			_fire.reset();
			spawn(_object, _object, v2<float>(), _direction);
		}
	}
	
	virtual void calculate(const float dt) {
		if (!_reaction.tick(dt))
			return;
		
		float range = getWeaponRange(_object);
		//LOG_DEBUG(("range = %g", range));

		_state.fire = false;

		const Object * result = NULL;
		float dist = -1;
		
		std::set<const Object *> objects;
		enumerateObjects(objects, range, &_targets);
		for(std::set<const Object *>::const_iterator i = objects.begin(); i != objects.end(); ++i) {
			const Object *target = *i;
			if (hasSameOwner(target) || target->aiDisabled())
				continue;
			
			v2<float> dpos = getRelativePosition(target);
			if (checkDistance(getCenterPosition(), target->getCenterPosition(), getZ(), true)) {
				if (result == NULL || dpos.quick_length() < dist) {
					result = target;
					dist = dpos.quick_length();
				}
			}
		}
		
		if (result != NULL) {
			_state.fire = true;
			_direction = getRelativePosition(result);
			_direction.normalize();
			//setDirection(_direction.getDirection(getDirectionsNumber()) - 1);
		}
	}
	
	virtual void onBreak() {
		Object *o = spawn("cannon-explosion", "cannon-explosion");
		o->setZ(getZ() + 1, true);
	}
private: 
	Alarm _reaction, _fire; 
	std::string _object;

	//no need to serialize it
	std::set<std::string> _targets;
};

REGISTER_OBJECT("pillbox", PillBox, ("machinegunner-bullet", true));
