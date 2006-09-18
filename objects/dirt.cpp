#include "object.h"
#include "resource_manager.h"

class Dirt : public Object {
public:
	Dirt() : Object("dirt") {}
	virtual Object * clone() const;
	virtual void onSpawn();
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);
};

void Dirt::onSpawn() {
	setDirection(0);
	setZ(-0.1);
	play("fade-in", false);
	play("main", true);
}

void Dirt::emit(const std::string &event, BaseObject * emitter) {
	if (event == "collision") {
		return;
	} else Object::emit(event, emitter);
}


Object* Dirt::clone() const  {
	Object *a = new Dirt(*this);
	return a;
}

REGISTER_OBJECT("dirt", Dirt, ());
