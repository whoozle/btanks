#include "object.h"
#include "mrt/logger.h"

Object::Object() : mass(1), speed(1), ttl(-1), dead (false) {
	_vx = _vy = _vz = _x = _y = _z = w = h = 0;
}

Object::~Object() {}

void Object::getPosition(float &x, float &y, float &z) {
	x = _x; y = _y; z = _z;
}

void Object::emit(const std::string &event, const Object * emitter) {
	if (event == "death") {
		_vx = _vy = _vz = 0;
		dead = true;
	} else LOG_WARN(("unhandled event '%s'", event.c_str()));
}

