#include "object.h"
#include "mrt/logger.h"
#include "world.h"

Object::Object(const std::string &classname)
 : mass(1), speed(1), ttl(-1), piercing(false), classname(classname), _direction(1,0,0), dead(false) {
	_velocity.clear();
	_position.clear();
}

Object::~Object() {}

void Object::getPosition(v3<float> &position) {
	position = _position;
}

void Object::emit(const std::string &event, const Object * emitter) {
	if (event == "death") {
		_velocity.clear();
		dead = true;
	} else LOG_WARN(("unhandled event '%s'", event.c_str()));
}

void Object::spawn(Object *o, const v3<float> &dpos, const v3<float> &vel) {
	World->spawn(this, o, dpos, vel);
}
