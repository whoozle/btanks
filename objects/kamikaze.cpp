#include "object.h"
#include "resource_manager.h"

class Kamikaze : public Object {
public:
	Kamikaze() : 
		Object("kamikaze") {}
	
	virtual void tick(const float dt);
	virtual void calculate(const float dt);

	virtual Object * clone() const;
	virtual void onSpawn();
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);
};

void Kamikaze::calculate(const float dt) {
	v3<float> vel;
	if (getNearest("player", _velocity, vel, NULL)) {
		_velocity.quantize8();
		int dir = _velocity.getDirection8();
		if (dir)
			setDirection(dir-1);
	} else _velocity.clear();
}

void Kamikaze::tick(const float dt) {
	const std::string state = getState();
	if (_velocity.is0()) {
		if (state != "hold") {
			cancelAll();
			play("hold", true);
		}
	} else {
		if (state == "hold") {
			cancelAll();
			play("run", true);
		}		
	}
	Object::tick(dt);
}

void Kamikaze::onSpawn() {
	play("hold", true);
}

void Kamikaze::emit(const std::string &event, BaseObject * emitter) {
	Object::emit(event, emitter);
}


Object* Kamikaze::clone() const  {
	Object *a = new Kamikaze(*this);
	return a;
}

REGISTER_OBJECT("kamikaze", Kamikaze, ());
