#include "object.h"
#include "mrt/logger.h"
#include "world.h"

Object::Object() : mass(1), speed(1), ttl(-1), piercing(false), dead (false)  {
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

void Object::spawn(Object *o, const float dx, const float dy, const float dz, const float vx, const float vy, const float vz) {
	World->spawn(this, o, dx, dy, dz, vx, vy, vz);
}
