#include "object.h"
#include "resource_manager.h"

class Rocket : public Object {
public:
	std::string type;
	Rocket(const std::string &type) : Object("rocket"), type(type) {}
	virtual void calculate(const float dt);
	virtual Object * clone() const;
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);
	void onSpawn();
};

void Rocket::onSpawn() {
	play("main", true);
	Object *_fire = spawnGrouped("single-pose", "rocket-fire", v3<float>(0,0, -0.1), Centered);
	_fire->impassability = 0;
	add("fire", _fire);
	
	_velocity.normalize();
	int dir = _velocity.getDirection16();
	if (dir) {
		setDirection(dir - 1);
	}
}

void Rocket::calculate(const float dt) {
	if (type == "guided") {
		v3<float> pos, vel;
		if (getNearest("player", pos, vel, NULL)) {
			_velocity = pos;
		}
		limitRotation(dt, 16, 0.2, false, false);
	}
}

void Rocket::emit(const std::string &event, BaseObject * emitter) {
	if (event == "death") {
		groupEmit("fire", "death");
		Object::emit("death", emitter);
	} else if (event == "collision") {
		spawn("explosion", "rocket-explosion", v3<float>(0,0,1), v3<float>(0,0,0));
		emit("death", emitter);
	} else Object::emit(event, emitter);
}


Object* Rocket::clone() const  {
	return new Rocket(*this);
}

REGISTER_OBJECT("guided-rocket", Rocket, ("guided"));
REGISTER_OBJECT("dumb-rocket", Rocket, ("dumb"));
REGISTER_OBJECT("smoke-rocket", Rocket, ("smoke"));
