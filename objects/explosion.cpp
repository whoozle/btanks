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
	if (!getState().size()) {	
		//LOG_DEBUG(("over"));
		emit("death", this);
	}
	AnimatedObject::tick(dt);
}

void Explosion::emit(const std::string &event, const Object * emitter) {
	if (event == "collision") {
		return;
	} else Object::emit(event, emitter);
}


Object* Explosion::clone() const  {
	AnimatedObject *a = new Explosion;
	ResourceManager->initMe(a, "explosion");
	a->speed = 0;
	a->hp = 1000;
	a->setDirection(0);
	a->play("boom", false);
	a->impassability = 0;
	return a;
}

REGISTER_OBJECT("explosion", Explosion, ());
