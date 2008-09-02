#define _USE_MATH_DEFINES
#include <math.h>
#include "rotating_object.h"
#include "sdlx/surface.h"

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
	if (angle < 0)
		angle += M_PI * 2;
	//LOG_DEBUG(("angle = %g", angle));
	_velocity.x = dv * cos(angle);
	_velocity.y = - dv * sin(angle);
}

void RotatingObject::tick(const float dt) {
	int dirs = get_directions_number();
	int dir = (int)(angle * dirs / M_PI / 2 + 0.5);
	dir %= dirs;
	if (dir < 0)
		dir += dirs;
	//LOG_DEBUG(("dir: %g", dir));
	set_direction(dir);
	
	Object::tick(dt);
}
