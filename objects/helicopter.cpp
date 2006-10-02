#include "object.h"
#include "alarm.h"
#include "tmx/map.h"
#include "mrt/random.h"
#include "resource_manager.h"

#define SPAWN_RATE 1

class Helicopter : public Object {
public:
	Helicopter(const std::string &para) :
		 Object("helicopter"), _active(false), _spawn(SPAWN_RATE, true), _paratrooper(para) {}
	virtual void calculate(const float dt);
	virtual void tick(const float dt);
	virtual Object * clone() const;
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);
	void onSpawn();

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		_next_target.serialize(s);
		s.add(_active);
		s.add(_paratrooper);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		_next_target.deserialize(s);
		s.get(_active);
		s.get(_paratrooper);
	}

private: 
	v3<float> _next_target;
	bool _active;
	Alarm _spawn;
	std::string _paratrooper;
};

void Helicopter::onSpawn() {
	play("main", true);
}

void Helicopter::tick(const float dt) {
	Object::tick(dt);
	if (_active && _spawn.tick(dt)) {
		spawn(_paratrooper, "paratrooper", v3<float>(0,0,-1), v3<float>());
	}
}


void Helicopter::calculate(const float dt) {
	if (!_active && _idle_time > 2.0) { //2 seconds delay before random target
		v3<float> pos;
		getPosition(pos);

		v3<int> size = Map->getSize();
		_next_target.x = mrt::random(size.x);
		_next_target.y = mrt::random(size.y);
		//LOG_DEBUG(("picking up random target: %g %g", _next_target.x, _next_target.x));
		_active = true;
	}
	if (_active) {
		v3<float> pos;
		getPosition(pos);
		if (pos.quick_distance(_next_target) <= 10000) {
			_active = false; 
			_velocity.clear();
		} else 
			_velocity = _next_target - pos;
	}
	
	limitRotation(dt, 8, 0.2, true, false);
}

void Helicopter::emit(const std::string &event, BaseObject * emitter) {
	Object::emit(event, emitter);
}


Object* Helicopter::clone() const  {
	return new Helicopter(*this);
}

REGISTER_OBJECT("helicopter-with-kamikazes", Helicopter, ("paratrooper-kamikaze"));
