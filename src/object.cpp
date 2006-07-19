#include "object.h"

Object::Object() : mass(1), speed(1) {
	_vx = _vy = _vz = _x = _y = _z = w = h = 0;
}

Object::~Object() {}

void Object::getPosition(float &x, float &y, float &z) {
	x = _x; y = _y; z = _z;
}


