#include "object.h"
#include "resource_manager.h"

class Explosion : public Object {
public:
	Explosion() : Object("explosion") {}
	virtual void tick(const float dt);
	virtual Object * clone() const;
	virtual void onSpawn();
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);
};


void Explosion::tick(const float dt) {
	Object::tick(dt);
	if (getState().empty()) {	
		//LOG_DEBUG(("over"));
		emit("death", this);
	}
}

void Explosion::onSpawn() {
	setDirection(0);
	play("boom", false);
	impassability = 0;	
}

void Explosion::emit(const std::string &event, BaseObject * emitter) {
	if (event == "collision") {
		return;
	} else Object::emit(event, emitter);
}


Object* Explosion::clone() const  {
	Object *a = new Explosion(*this);
	return a;
}

REGISTER_OBJECT("explosion", Explosion, ());
