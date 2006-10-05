#include "object.h"
#include "resource_manager.h"

class Missile : public Object {
public:
	std::string type;
	Missile(const std::string &type) : Object("missile"), type(type) {}
	virtual void calculate(const float dt);
	virtual Object * clone() const;
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);
	void onSpawn();

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(type);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(type);
	}

};

void Missile::onSpawn() {
	play("main", true);
	Object *_fire = spawnGrouped("single-pose", "missile-fire", v3<float>(), Centered);
	_fire->impassability = 0;
	add("fire", _fire);
	
	_velocity.normalize();
	int dir = _velocity.getDirection16();
	if (dir) {
		setDirection(dir - 1);
	}
}

void Missile::calculate(const float dt) {
	if (type == "guided") {
		v3<float> pos, vel;
		if (getNearest("player", pos, vel, NULL)) {
			_velocity = pos;
		}
		if (getNearest("kamikaze", pos, vel, NULL)) {
			if (_velocity.quick_length() > pos.quick_length()) 
				_velocity = pos;
		}
		limitRotation(dt, 16, 0.2, false, false);
	}
}

void Missile::emit(const std::string &event, BaseObject * emitter) {
	if (event == "collision") {
		v3<float> dpos = getRelativePos(emitter) / 2;
		emit("death", emitter);
	} if (event == "death" && type == "smoke") {
		spawn("smoke-cloud", "smoke-cloud");
		Object::emit(event, emitter);
	} else if (event == "death" && type == "nuke") {
		spawn("nuclear-explosion", "nuclear-explosion");
		Object::emit(event, emitter);
	} else if (event == "death") {
		spawn("explosion", "missile-explosion");
		Object::emit(event, emitter);
	} else Object::emit(event, emitter);
}


Object* Missile::clone() const  {
	return new Missile(*this);
}

REGISTER_OBJECT("guided-missile", Missile, ("guided"));
REGISTER_OBJECT("dumb-missile", Missile, ("dumb"));
REGISTER_OBJECT("smoke-missile", Missile, ("smoke"));
REGISTER_OBJECT("nuke-missile", Missile, ("nuke"));
