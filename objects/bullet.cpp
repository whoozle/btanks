#include "object.h"
#include "resource_manager.h"

class Bullet : public Object {
public:
	Bullet() : Object("bullet") {}
	virtual void calculate(const float dt);
	virtual Object * clone() const;
	virtual void onSpawn();
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);
};


void Bullet::calculate(const float dt) {
	_velocity.normalize();
	
	int dir = _velocity.getDirection8();
	if (dir) {
		setDirection(dir - 1);
	}
	_velocity.quantize8();
}

void Bullet::onSpawn() {
/*	a->speed = 500;
	a->ttl = 1;
	a->piercing = true;
*/
	play("move", true);
}

void Bullet::emit(const std::string &event, BaseObject * emitter) {
	if (event == "collision") {
		Object::emit("death", emitter);
	} else Object::emit(event, emitter);
}


Object* Bullet::clone() const  {
	Object *a = new Bullet(*this);
	a->setDirection(getDirection());
	return a;
}

REGISTER_OBJECT("bullet", Bullet, ());
