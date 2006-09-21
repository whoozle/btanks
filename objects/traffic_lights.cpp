#include "object.h"
#include "resource_manager.h"

class TrafficLights : public Object {
public:
	TrafficLights() : Object("traffic-lights"), _idx(-1), _broken(false) {}
	virtual void tick(const float dt);
	virtual Object * clone() const;
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);
	virtual void addDamage(BaseObject *from, const bool emitDeath = true);

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_idx);
		s.add(_broken);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_idx);
		s.get(_broken);
	}

private: 
	int _idx;
	bool _broken;
};

void TrafficLights::addDamage(BaseObject *from, const bool emitDeath) {
	if (_broken)
		return;

	BaseObject::addDamage(from, false);
	if (hp <= 0) {
		_broken = true;
		cancelAll();
		play("fade-out", false); 
		play("broken", true);
	}
}

void TrafficLights::tick(const float dt) {
	Object::tick(dt);

	static const char *names[] = {"red", "flashing-red", "yellow", "green", "flashing-green", "yellow"};
	
	if (getState().empty()) {
		++_idx; 
		_idx %= sizeof(names) / sizeof(names[0]);

		//LOG_DEBUG(("tick! %d: %s", _idx, names[_idx]));
		play(names[_idx]);
	}
}

void TrafficLights::emit(const std::string &event, BaseObject * emitter) {
	Object::emit(event, emitter);
}


Object* TrafficLights::clone() const  {
	return new TrafficLights(*this);
}

REGISTER_OBJECT("traffic-lights", TrafficLights, ());
