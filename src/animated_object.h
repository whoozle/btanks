#ifndef __BTANKS_ANIMATED_OBJECT__
#define __BTANKS_ANIMATED_OBJECT__

#include "object.h"

namespace sdlx {
	class Surface;
}

class AnimatedObject : public Object {
public:
	AnimatedObject();
	AnimatedObject(sdlx::Surface *surface, const long tile_w, const long tile_h);
	
	virtual void tick(const float dt);
	
protected: 
	sdlx::Surface *_surface;
	long _poses, _fpp;
};

#endif

