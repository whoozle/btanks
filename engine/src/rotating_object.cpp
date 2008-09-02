#include <math.h>
#include "rotating_object.h"

RotatingObject::RotatingObject(const std::string &classname) : Object(classname), angle_speed(0) {

}

void RotatingObject::calculate(const float dt) {
	if (_parent != NULL) {
		Object::tick(dt);
		return;
	}
	
	_velocity.clear();
	int dv = (_state.up?1: 0) + (_state.down?-1:0);
	if (dv == 0) //fix me later
		return;
	
	angle = fmodf(angle + angle_speed * dt * ((_state.right? -1: 0) + (_state.left? 1: 0)), M_PI * 2);
	LOG_DEBUG(("angle = %g", angle));
	_velocity.x = dv * cos(angle);
	_velocity.y = dv * sin(angle);
}
