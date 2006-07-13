#include "object.h"

Object::Object() : mass(1) {}
Object::~Object() {}

void Object::setV(const float vx, const float vy) {
	_vx = vx;
	_vy = vy;
}
