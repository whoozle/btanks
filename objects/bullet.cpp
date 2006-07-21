#include "animated_object.h"
#include "resource_manager.h"

class Bullet : public AnimatedObject {
public:
	Bullet() : AnimatedObject("bullet") {
	}
	Bullet(const Bullet &other) : AnimatedObject(other.classname) {
		ResourceManager->initMe(this, "bullet");
	}
//	virtual void tick(float) {}
//	virtual void render(sdlx::Surface&, int, int) {}
};

REGISTER_OBJECT("bullet", Bullet, ());
