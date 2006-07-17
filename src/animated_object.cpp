#include "animated_object.h"
#include "sdlx/surface.h"
#include "mrt/logger.h"

AnimatedObject::AnimatedObject() : _surface(0) {}
AnimatedObject::AnimatedObject(sdlx::Surface *surface, const int tile_w, const int tile_h, const float speed) {
	_surface = surface;
	_tw = tile_w; _th = tile_h;
	_poses = (surface->getWidth()-1)/tile_w + 1;
	_fpp = (surface->getHeight()-1)/tile_w + 1;
	_pose= 0;
	_speed = speed;
	_pos = 0;
	LOG_DEBUG(("poses: %d, fpp: %d, speed: %f", _poses, _fpp, _speed));
}

void AnimatedObject::setPose(const int pose) {
	_pose = pose;
}


void AnimatedObject::tick(const float dt) {
	_pos += dt * _speed;
	_pos -= _fpp * ((int)_pos / _fpp);
}

void AnimatedObject::render(sdlx::Surface &surface, const int x, const int y) {
	int frame = (int)_pos;
	sdlx::Rect src(_pose * _tw, frame * _th, _tw, _th);
	surface.copyFrom(*_surface, src, x, y);
}
