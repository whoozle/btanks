#include "object.h"
#include "resource_manager.h"

class Explosion : public Object {
public:
	Explosion() : Object("explosion") {}
	virtual void tick(const float dt);
	virtual Object * clone(const std::string &opt) const;
	virtual void emit(const std::string &event, const BaseObject * emitter = NULL);
};


void Explosion::tick(const float dt) {
	Object::tick(dt);
	if (!getState().size()) {	
		//LOG_DEBUG(("over"));
		emit("death", this);
	}
}

void Explosion::emit(const std::string &event, const BaseObject * emitter) {
	if (event == "collision") {
		return;
	} else Object::emit(event, emitter);
}


Object* Explosion::clone(const std::string &opt) const  {
	Object *a = new Explosion(*this);
	ResourceManager->initMe(a, opt);
/*	a->speed = 0;
	a->hp = 1000;
*/	a->setDirection(0);
	a->play("boom", false);
	a->impassability = 0;
	return a;
}

REGISTER_OBJECT("explosion", Explosion, ());
