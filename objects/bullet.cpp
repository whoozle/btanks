#include "animated_object.h"
#include "resource_manager.h"

class Bullet : public AnimatedObject {
public:
	Bullet() : AnimatedObject("bullet") {}
	virtual void tick(const float dt);
	virtual Object * clone() const;
	virtual void emit(const std::string &event, const Object * emitter = NULL);
};


void Bullet::tick(const float dt) {
	_velocity.normalize();
	int dir = v3<float>::getDirection8(_velocity);
	if (dir) {
		setDirection(dir - 1);
	}
	AnimatedObject::tick(dt);
}

void Bullet::emit(const std::string &event, const Object * emitter) {
	if (event == "collision") {
		emit("death", this);
	} else Object::emit(event, emitter);
}


Object* Bullet::clone() const  {
	AnimatedObject *a = new Bullet;
	ResourceManager->initMe(a, "bullet");
	a->speed = 500;
	a->ttl = 1;
	a->piercing = true;
	a->play("move", true);
	return a;
}

REGISTER_OBJECT("bullet", Bullet, ());
