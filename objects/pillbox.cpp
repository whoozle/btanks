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
#include "config.h"
#include "destructable_object.h"
#include "mrt/random.h"
#include "ai/base.h"
#include "ai/targets.h"
#include "special_owners.h"

class PillBox : public DestructableObject, protected ai::Base {
	Alarm _reaction, _fire; 
	std::string _object;
public: 
	PillBox(const std::string &object) : 
		DestructableObject("pillbox"), _reaction(true), _fire(false), _object(object) {}

	virtual Object * clone() const { return new PillBox(*this); }
	
	virtual void onSpawn() { 
		GET_CONFIG_VALUE("objects.pillbox.reaction-time", float, rt, 0.1);
		mrt::randomize(rt, rt / 2);
		_reaction.set(rt);

		GET_CONFIG_VALUE("objects.pillbox.fire-rate", float, fr, 0.2);
		_fire.set(fr);
		
		DestructableObject::onSpawn();
		ai::Base::onSpawn(this);
		ai::Base::multiplier = 5.0f;
	}

	virtual void serialize(mrt::Serializator &s) const {
		DestructableObject::serialize(s);
		ai::Base::serialize(s);
		s.add(_reaction);
		s.add(_fire);
		s.add(_object);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		DestructableObject::deserialize(s);
		ai::Base::deserialize(s);
		s.get(_reaction);
		s.get(_fire);
		s.get(_object);
	}
	
	virtual void tick(const float dt) {
		Object::tick(dt);
		if (!_broken && _state.fire) {

			bool fire = false;
			if (_fire.tick(dt)) {
				_fire.reset();
				if (canFire()) {
					fire = true;
					spawn(_object, _object, v2<float>(), _direction);
				} 
			}
			
			int dirs = 16/* bullet->getDirectionsNumber() */, d = _direction.getDirection(dirs);
			v2<float> dpos; 
			dpos.fromDirection((d + dirs / 4) % dirs, dirs);
			dpos *= 16;

			if (fire) {
				spawn(_object, _object, dpos, _direction);
				spawn(_object, _object, -dpos, _direction);
			}
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
		enumerateObjects(objects, range, &ai::Targets->troops );
		for(std::set<const Object *>::const_iterator i = objects.begin(); i != objects.end(); ++i) {
			const Object *target = *i;
			if (hasSameOwner(target) || target->aiDisabled() || target->pierceable || target->impassability == 0)
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
		Object *o = spawn("explosion", "cannon-explosion");
		o->setZ(getZ() + 1, true);
		for(int i = 0; i < 2; ++i) {
			o = spawn("machinegunner", "machinegunner", size / 2);
			o->copy_special_owners(this);
		}
	}
};

REGISTER_OBJECT("pillbox", PillBox, ("machinegunner-bullet"));
