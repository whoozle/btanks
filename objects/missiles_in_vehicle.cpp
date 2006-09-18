#include "object.h"
#include "resource_manager.h"
#include "world.h"

class MissilesInVehicle : public Object {
public:
	MissilesInVehicle(const std::string &vehicle, const int n) : Object("missiles-in-vehicle"), n(n), max_n(n), hold(true), _vehicle(vehicle) {}
	virtual void tick(const float dt);
	virtual Object * clone() const;
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);
	virtual void onSpawn();
	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual const bool take(const BaseObject *obj, const std::string &type);
	
	void updatePose();
private:
	int n, max_n;
	bool hold;
	std::string _vehicle, _object, _type;
};

const bool MissilesInVehicle::take(const BaseObject *obj, const std::string &type) {
	if (obj->classname == "missiles" || obj->classname == "mines") {
		_object = obj->classname;
		_type = type;
		std::string animation = type + "-" + obj->classname + "-on-" + _vehicle;
		setup(animation);
		n = max_n;
		
		if (type == "nuke") {
			if (n > 2)
				n = 2; //fixme
		}
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
	std::string pose = mrt::formatString("missile-%d%s", n, hold?"-hold":"");
	//LOG_DEBUG(("updating pose to '%s'", pose.c_str()));
	play(pose, true);
}

void MissilesInVehicle::onSpawn() {
	updatePose();
	impassability = 0;
}

void MissilesInVehicle::render(sdlx::Surface &surface, const int x, const int y) {
	if (n == 0) 
		return;
	Object::render(surface, x, y);
}

void MissilesInVehicle::tick(const float dt) {
	Object::tick(dt);
}

void MissilesInVehicle::emit(const std::string &event, BaseObject * emitter) {
	if (event == "move") {
		hold = false;
		updatePose();
	} else if (event == "hold") {
		hold = true;
		updatePose();
	} else if (event == "launch") {
		if (n > 0) {
			--n;
			LOG_DEBUG(("launching missile!"));
			{
				v3<float> v = _velocity.is0()?_direction:_velocity;
				v.normalize();
				std::string type = _type.empty()?"guided":_type;
				std::string object = _object.empty()?"missiles":_object;
				object = object.substr(0, object.size()-1); //remove trailing 's' 
				v3<float> opos(0,0,1);
				if (_object == "mines") opos.z = -1;
				World->spawn(dynamic_cast<Object *>(emitter), type + "-" + object, type + "-" + object, opos, v);
				
				if (_object != "mines") {
					const Object * la = ResourceManager.get_const()->getAnimation("missile-launch");
					v3<float> dpos = (size - la->size).convert<float>();
					dpos.z = 1;
					dpos /= 2;
		
					Object *o = World->spawn(dynamic_cast<Object *>(emitter), "missile-launch", "missile-launch", dpos, _direction);
					o->setDirection(getDirection());
				}
				//LOG_DEBUG(("dir: %d", o->getDirection()));	
			}
			updatePose();
		}
	} else if (event == "reload") {
		n = 3;
		updatePose();
	} else if (event == "collision") {
		return;
	} else Object::emit(event, emitter);
}


Object* MissilesInVehicle::clone() const  {
	return new MissilesInVehicle(*this);
}

REGISTER_OBJECT("missiles-on-launcher", MissilesInVehicle, ("launcher", 3));
REGISTER_OBJECT("missiles-on-tank", MissilesInVehicle, ("tank", 1));
