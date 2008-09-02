#define _USE_MATH_DEFINES
#include <math.h>
#include "rotating_object.h"
#include "sdlx/surface.h"

RotatingObject::RotatingObject(const std::string &classname) : Object(classname), angle_speed(0), angle(0), cached_angle(0), 
	cached_surface(NULL), src_surface(NULL) {}

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
	//LOG_DEBUG(("dir: %d, dd: %+g", dir, dd));
	set_direction(dir);
	
	Object::tick(dt);
}

void RotatingObject::render(sdlx::Surface &surface, const int x, const int y) {
	int dirs = get_directions_number();
/*	if (dirs >= ROTATE_STEPS) {
		Object::render(surface, x, y);
		return;
	}
*/
	if (angle == cached_angle && cached_surface != NULL) {
		//render cached copy
		surface.blit(*cached_surface, x + (int)size.x - cached_surface->get_width(), y + (int)size.y - cached_surface->get_height());
		return;
	}

	sdlx::Rect src;
	if (!get_render_rect(src))
		return;

	int dir = (int)(angle * dirs / M_PI / 2 + 0.5);
	float dd = angle - dir * (M_PI * 2 / dirs);

	if (angle != cached_angle || cached_surface == NULL) {
		if (cached_surface == NULL) {
			cached_surface = new sdlx::Surface;
		}

		if (src_surface == NULL) {
			src_surface = new sdlx::Surface;
			src_surface->create_rgb((int)size.x, (int)size.y, 32);
			src_surface->display_format_alpha();
		}

		const_cast<sdlx::Surface *>(_surface)->set_alpha(0,0);
		Object::render(*src_surface, 0, 0); 
		const_cast<sdlx::Surface *>(_surface)->set_alpha(0);

		cached_surface->rotozoom(*src_surface, dd, 1, true);
		cached_surface->display_format_alpha();

		cached_angle = angle;
	}
}

RotatingObject::~RotatingObject() {
	delete cached_surface;
	delete src_surface;
}
