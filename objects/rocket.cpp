#include "object.h"
#include "resource_manager.h"

class Rocket : public Object {
public:
	Rocket() : Object("bullet") {}
	virtual void calculate(const float dt);
	virtual Object * clone(const std::string &opt) const;
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);
	void onSpawn();
private:
	Object *_fire;
};

void Rocket::onSpawn() {
	play("main", true);
	_fire = spawnGrouped("single-pose", "rocket-fire", v3<float>(0,0, -0.1), Centered);
	_fire->impassability = 0;
}

void Rocket::calculate(const float dt) {
	v3<float> pos, vel;
	if (getNearest("player", pos, vel, NULL)) {
		_velocity = pos;
	}

	_velocity.normalize();
	_velocity.quantize16();
	limitRotation(dt, 16, 0.5, true);
}

void Rocket::emit(const std::string &event, BaseObject * emitter) {
	if (event == "death") {
		_fire->emit("death", this);
		Object::emit("death", emitter);
	} else if (event == "collision") {
		emit("death", emitter);
	} else Object::emit(event, emitter);
}


Object* Rocket::clone(const std::string &opt) const  {
	Object *a = new Rocket(*this);
	ResourceManager->initMe(a, opt);

	a->setDirection(getDirection());
	return a;
}

REGISTER_OBJECT("rocket", Rocket, ());
