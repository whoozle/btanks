#include "animated_object.h"
#include "resource_manager.h"

class Bullet : public Object {
public:
	Bullet() : Object("bullet") {}
	virtual void tick(float) {}
	virtual void render(sdlx::Surface&, int, int) {}
};

REGISTER_OBJECT("bullet", Bullet, ());
