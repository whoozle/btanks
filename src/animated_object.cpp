#include "animated_object.h"

AnimatedObject::AnimatedObject() : _surface(0) {}
AnimatedObject::AnimatedObject(sdlx::Surface *surface, const long tile_w, const long tile_h, const long w, const long h) : 
	_surface(surface) {}
