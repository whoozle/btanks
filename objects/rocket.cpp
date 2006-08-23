#include "object.h"
#include "resource_manager.h"

class Rocket : public Object {
public:
	Rocket() : Object("bullet", true) {}
	virtual void tick(const float dt);
	virtual Object * clone(const std::string &opt) const;
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);
	void onSpawn();
};

void Rocket::onSpawn() {
	play("main", true);
}


void Rocket::tick(const float dt) {
	Object::tick(dt);

	v3<float> pos, vel;
	if (getNearest("player", pos, vel, NULL)) {
		_velocity = pos;
	}

	_velocity.normalize();
	int dir = v3<float>::getDirection8(_velocity);
	if (dir > 0)
		setDirection(dir - 1);
}

void Rocket::emit(const std::string &event, BaseObject * emitter) {
	if (event == "collision") {
		Object::emit("death", emitter);
	} else Object::emit(event, emitter);
}


Object* Rocket::clone(const std::string &opt) const  {
	Object *a = new Rocket(*this);
	ResourceManager->initMe(a, opt);

	a->setDirection(getDirection());
	return a;
}

REGISTER_OBJECT("rocket", Rocket, ());
