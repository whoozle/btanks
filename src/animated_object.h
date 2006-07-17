#ifndef __BTANKS_ANIMATED_OBJECT__
#define __BTANKS_ANIMATED_OBJECT__

#include "object.h"

namespace sdlx {
	class Surface;
}

class AnimatedObject : public Object {
public:
	AnimatedObject();
	AnimatedObject(sdlx::Surface *surface, const long tile_w, const long tile_h, const long w, const long h);
	
protected: 
	sdlx::Surface *_surface;
};

#endif

