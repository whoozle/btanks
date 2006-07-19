#include "object.h"
#include "mrt/logger.h"
#include "world.h"

Object::Object() : mass(1), speed(1), ttl(-1), piercing(false), dead (false)  {
	_velocity.clear();
	_position.clear();
	w = h = 0;
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
