#include "object.h"
#include "resource_manager.h"
#include "world.h"

class RocketsInVehicle : public Object {
public:
	RocketsInVehicle(const int n) : Object("rockets-in-vehicle"), n(n), hold(true) {}
	virtual void tick(const float dt);
	virtual Object * clone() const;
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);
	virtual void onSpawn();
	virtual void render(sdlx::Surface &surface, const int x, const int y);
	
	void updatePose();
private:
	int n;
	bool hold;
};


void RocketsInVehicle::updatePose() {
	if (n <= 0)
		return;
	cancelAll();
	std::string pose = mrt::formatString("rocket-%d%s", n, hold?"-hold":"");
	//LOG_DEBUG(("updating pose to '%s'", pose.c_str()));
	play(pose, true);
}

void RocketsInVehicle::onSpawn() {
	updatePose();
	impassability = 0;
}

void RocketsInVehicle::render(sdlx::Surface &surface, const int x, const int y) {
	if (n == 0) 
		return;
	Object::render(surface, x, y);
}

void RocketsInVehicle::tick(const float dt) {
	Object::tick(dt);
}

void RocketsInVehicle::emit(const std::string &event, BaseObject * emitter) {
	if (event == "move") {
		hold = false;
		updatePose();
	} else if (event == "hold") {
		hold = true;
		updatePose();
	} else if (event == "launch") {
		if (n > 0) {
			--n;
			LOG_DEBUG(("launching rocket!"));
			{
				v3<float> v = _velocity.is0()?_direction:_velocity;
				v.normalize();
				World->spawn(dynamic_cast<Object *>(emitter), "guided-rocket", "guided-rocket", v3<float>(0,0,1), v);
				
				const Object * la = ResourceManager.get_const()->getAnimation("rocket-launch");
				v3<float> dpos = (size - la->size).convert<float>();
				dpos.z = 1;
				dpos /= 2;

				Object *o = World->spawn(dynamic_cast<Object *>(emitter), "rocket-launch", "rocket-launch", dpos, _direction);
				o->setDirection(getDirection());
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


Object* RocketsInVehicle::clone() const  {
	return new RocketsInVehicle(*this);
}

REGISTER_OBJECT("rockets-on-launcher", RocketsInVehicle, (3));
REGISTER_OBJECT("rockets-on-tank", RocketsInVehicle, (1));
