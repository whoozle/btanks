#include "object.h"
#include "resource_manager.h"

class TrafficLights : public Object {
public:
	TrafficLights() : Object("traffic-lights"), _idx(-1) {}
	virtual void tick(const float dt);
	virtual Object * clone() const;
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);
private: 
	int _idx;
};


void TrafficLights::tick(const float dt) {
	Object::tick(dt);

	static const char *names[] = {"red", "flashing-red", "yellow", "green", "flashing-green", "yellow"};
	
	if (getState().size() == 0) {
		++_idx; 
		_idx %= sizeof(names) / sizeof(names[0]);

		//LOG_DEBUG(("tick! %d: %s", _idx, names[_idx]));
		play(names[_idx]);
	}
}

void TrafficLights::emit(const std::string &event, BaseObject * emitter) {
	if (event != "collision")
		Object::emit(event, emitter);
}


Object* TrafficLights::clone() const  {
	return new TrafficLights(*this);
}

REGISTER_OBJECT("traffic-lights", TrafficLights, ());
