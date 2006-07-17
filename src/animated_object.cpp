#include "animated_object.h"
#include "sdlx/surface.h"
#include "mrt/logger.h"

AnimatedObject::AnimatedObject() : _surface(0) {}
AnimatedObject::AnimatedObject(sdlx::Surface *surface, const long tile_w, const long tile_h, const float speed) {
	_surface = surface;
	_tw = tile_w; _th = tile_h;
	_poses = (surface->getWidth()-1)/tile_w + 1;
	_fpp = (surface->getHeight()-1)/tile_w + 1;
	_pose= 0;
	_speed = speed;
	_pos = 0;
	LOG_DEBUG(("poses: %ld, fpp: %ld", _poses, _fpp));
}

void AnimatedObject::tick(const float dt) {
	_pos += dt * _speed;
	_pos = _fpp * ((long)_pos / _fpp);
}

void AnimatedObject::render(sdlx::Surface &surface, const int x, const int y) {
	long frame = (long)_pos;
	sdlx::Rect src(_pose * _tw, frame * _th, _tw, _th);
	surface.copyFrom(*_surface, src, x, y);
}
