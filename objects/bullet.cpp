#include "object.h"
#include "resource_manager.h"

class Bullet : public Object {
public:
	Bullet() : Object("bullet", true) {}
	virtual void tick(const float dt);
	virtual Object * clone(const std::string &opt) const;
	virtual void emit(const std::string &event, const BaseObject * emitter = NULL);
};


void Bullet::tick(const float dt) {
	_velocity.normalize();
	int dir = v3<float>::getDirection8(_velocity);
	if (dir) {
		setDirection(dir - 1);
	}
	Object::tick(dt);
}

void Bullet::emit(const std::string &event, const BaseObject * emitter) {
	if (event == "collision") {
		Object::emit("death", emitter);
	} else Object::emit(event, emitter);
}


Object* Bullet::clone(const std::string &opt) const  {
	Object *a = new Bullet(*this);
	ResourceManager->initMe(a, opt);
/*	a->speed = 500;
	a->ttl = 1;
	a->piercing = true;
*/
	a->play("move", true);
	a->setDirection(getDirection());
	return a;
}

REGISTER_OBJECT("bullet", Bullet, ());
