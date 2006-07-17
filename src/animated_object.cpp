#include "animated_object.h"
#include "sdlx/surface.h"
#include "mrt/logger.h"

AnimatedObject::AnimatedObject() : _surface(0) {}
AnimatedObject::AnimatedObject(sdlx::Surface *surface, const long tile_w, const long tile_h):
	_surface(surface), _poses((surface->getWidth()-1)/tile_w + 1), _fpp((surface->getHeight()-1)/tile_w + 1) {
	LOG_DEBUG(("poses: %ld, fpp: %ld", _poses, _fpp));
}

void AnimatedObject::tick(const float dt) {}
