#include "animated_object.h"
#include "resource_manager.h"

class Explosion : public AnimatedObject {
public:
	Explosion() : AnimatedObject("explosion") {}
	virtual void tick(const float dt);
	virtual Object * clone() const;
	virtual void emit(const std::string &event, const Object * emitter = NULL);
};


void Explosion::tick(const float dt) {
	if (getState() == "")	
		emit("death", this);
}

void Explosion::emit(const std::string &event, const Object * emitter) {
	if (event == "collision") {
		emit("death", this);
	} else Object::emit(event, emitter);
}


Object* Explosion::clone() const  {
	AnimatedObject *a = new Explosion;
	ResourceManager->initMe(a, "Explosion");
	a->speed = 0;
	a->setDirection(0);
	a->play("boom", false);
	return a;
}

REGISTER_OBJECT("explosion", Explosion, ());
