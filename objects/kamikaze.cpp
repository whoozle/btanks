#include "object.h"
#include "resource_manager.h"
#include "alarm.h"

class Kamikaze : public Object {
public:
	Kamikaze() : 
		Object("kamikaze"), _reaction(0.1, true) {}
	
	virtual void tick(const float dt);
	virtual void calculate(const float dt);

	virtual Object * clone() const;
	virtual void onSpawn();
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);
private: 
	Alarm _reaction;
};

void Kamikaze::calculate(const float dt) {
	if (!_reaction.tick(dt))
		return;
	
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
	if (event == "death") {
		spawn("explosion", "missile-explosion", v3<float>(0,0,1), v3<float>());
		Object::emit(event, emitter);
	} else if (event == "collision") {
		if (emitter) 
			emitter->addDamage(this, hp);
		emit("death", emitter);
	} else 
		Object::emit(event, emitter);
}


Object* Kamikaze::clone() const  {
	Object *a = new Kamikaze(*this);
	return a;
}

REGISTER_OBJECT("kamikaze", Kamikaze, ());
