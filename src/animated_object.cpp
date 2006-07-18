#include "animated_object.h"
#include "sdlx/surface.h"
#include "mrt/logger.h"
#include "animation_model.h"

AnimatedObject::AnimatedObject(AnimationModel *model, sdlx::Surface *surface, const int tile_w, const int tile_h) {
	_model = model;
	_surface = surface;
	_tw = tile_w; _th = tile_h;
	_poses = (surface->getWidth()-1)/tile_w + 1;
	_fpp = (surface->getHeight()-1)/tile_w + 1;
	_pose= 0;
	_pos = 0;
	LOG_DEBUG(("poses: %d, fpp: %d", _poses, _fpp));
	_active = false;
	_repeat = false;
}

void AnimatedObject::setPose(const int pose) {
	_pose = pose;
}

const int AnimatedObject::getPose() const {
	return _pose;
}

void AnimatedObject::play(const bool repeat) {
	_repeat = repeat;
	_active = true;
	_pos = 0;
}


void AnimatedObject::stop() {
	_active = false;
	_pos = 0;
}


void AnimatedObject::tick(const float dt) {
	if (!_active) 
		return;
	
	_pos += dt /* _speed*/;
	int cycles = ((int)_pos / _fpp);
	if (cycles && !_repeat) 
		stop();
	_pos -= cycles * _fpp;
}

void AnimatedObject::render(sdlx::Surface &surface, const int x, const int y) {
	int frame = (int)_pos;
	sdlx::Rect src(_pose * _tw, frame * _th, _tw, _th);
	surface.copyFrom(*_surface, src, x, y);
}
