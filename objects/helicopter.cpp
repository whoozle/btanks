#include "object.h"
#include "tmx/map.h"
#include "mrt/random.h"
#include "resource_manager.h"

class Helicopter : public Object {
public:
	Helicopter() : Object("helicopter"), _active(false) {}
	virtual void calculate(const float dt);
	virtual Object * clone() const;
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);
	void onSpawn();

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		_next_target.serialize(s);
		s.add(_active);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		_next_target.deserialize(s);
		s.get(_active);
	}

private: 
	v3<float> _next_target;
	bool _active;
};

void Helicopter::onSpawn() {
	play("main", true);
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

REGISTER_OBJECT("helicopter", Helicopter, ());
