
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
#include "world.h"
#include "config.h"
#include "vehicle_traits.h"

class MissilesInVehicle : public Object {
public:
	void update() {
		need_sync = true;
		if (_object.empty() || _type.empty()) {
			if (_install_default) {
				if (_object.empty())
					Config->get("objects." + registered_name + ".default-weapon", _object, "missiles");
				if (_type.empty())
					Config->get("objects." + registered_name + ".default-weapon-type", _type, "guided");
			} else {
				max_n = n = 0; 
				return;
			}
		}
		VehicleTraits::getWeaponCapacity(max_n, max_v, _vehicle, _object, _type);
		n = max_n;
	}

	MissilesInVehicle(const std::string &vehicle, bool install_default = true) : 
		Object("missiles-on-vehicle"), n(0), max_v(0), max_n(0), hold(true),  _vehicle(vehicle), _install_default(install_default) {
		// _object(object), _type(type)
		impassability = 0;
		hp = -1;
	}
	
	virtual const std::string getType() const {
		if (_object.empty() && _type.empty()) 
			return std::string();
		
		return _object + ":" + _type;
	}

	virtual const int getCount() const {
		return n;
	}
	
	virtual Object * clone() const;
	virtual void emit(const std::string &event, Object * emitter = NULL);
	virtual void onSpawn();

	virtual const bool skipRendering() const;
	virtual const bool take(const BaseObject *obj, const std::string &type);
	
	void updatePose();
	
	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(n);
		s.add(max_n);
		s.add(max_v);
		s.add(hold);
		s.add(_vehicle);
		s.add(_object);
		s.add(_type);
		s.add(_install_default);
	}

	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(n);
		s.get(max_n);
		s.get(max_v);
		s.get(hold);
		s.get(_vehicle);
		s.get(_object);
		s.get(_type);
		s.get(_install_default);
		//LOG_DEBUG(("deserialize. max: %d, max visible: %d, quantity: %d", max_n, max_v, n));
		updatePose();
		//LOG_DEBUG(("update. max: %d, max visible: %d, quantity: %d", max_n, max_v, n));
		//LOG_DEBUG(("z = %d", getZ()));
	}
	
private:
	int n, max_v, max_n;
	bool hold;
	std::string _vehicle, _object, _type;
	
	//serialization is not needed: 
	bool _install_default;
};

const bool MissilesInVehicle::skipRendering() const {
	if (n == 0)
		return true;
	return Object::skipRendering();
}


const bool MissilesInVehicle::take(const BaseObject *obj, const std::string &type) {
	if (obj->classname == "missiles" || obj->classname == "mines") {
		_object = obj->classname;
		_type = type;
		std::string animation = type + "-" + obj->classname + "-on-" + _vehicle;
		init(animation);
		update();
		updatePose();
		LOG_DEBUG(("missiles : %s taken", type.c_str()));
		return true;
	}
	return false;
}

void MissilesInVehicle::updatePose() {
	if (n <= 0)
		return;
	cancelAll();
	std::string pose = mrt::formatString("missile-%d%s", (n > max_v)?max_v:n, hold?"-hold":"");
	//LOG_DEBUG(("updating pose to '%s'", pose.c_str()));
	play(pose, true);
}

void MissilesInVehicle::onSpawn() {
	update();
	updatePose();
}

void MissilesInVehicle::emit(const std::string &event, Object * emitter) {
	if (event == "move") {
		hold = false;
		updatePose();
	} else if (event == "hold") {
		hold = true;
		updatePose();
	} else if (event == "launch") {
		if (n > 0) {
			--n;
			//LOG_DEBUG(("launching missile!"));
			{
				v2<float> v = _velocity.is0()?_direction:_velocity;
				v.normalize();
				std::string object = _object.substr(0, _object.size() - 1); //remove trailing 's' 
				World->spawn(emitter, _type + "-" + object, _type + "-" + object, v2<float>::empty, v);
				
/*				if (_object != "mines") {
					const Object * la = ResourceManager.get_const()->getAnimation("missile-launch");
					v2<float> dpos = (size - la->size).convert<float>();
					dpos.z = 0;
					dpos /= 2;
		
					Object *o = World->spawn(emitter, "missile-launch", "missile-launch", dpos, v2<float>::empty);
					o->setDirection(getDirection());
				}
*/
				//LOG_DEBUG(("dir: %d", o->getDirection()));	
			}
			updatePose();
		}
	} else if (event == "reload") {
		n = max_n;
		updatePose();
	} else if (event == "collision") {
		return;
	} else Object::emit(event, emitter);
}


Object* MissilesInVehicle::clone() const  {
	return new MissilesInVehicle(*this);
}

REGISTER_OBJECT("missiles-on-launcher", MissilesInVehicle, ("launcher"));
REGISTER_OBJECT("alt-missiles-on-launcher", MissilesInVehicle, ("launcher", false));
REGISTER_OBJECT("missiles-on-tank", MissilesInVehicle, ("tank"));
REGISTER_OBJECT("missiles-on-boat", MissilesInVehicle, ("boat"));
