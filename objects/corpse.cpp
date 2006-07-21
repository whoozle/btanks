#include "animated_object.h"
#include "resource_manager.h"

class Corpse : public AnimatedObject {
public:
	Corpse() : AnimatedObject("corpse") {}
	virtual Object * clone() const;
	virtual void emit(const std::string &event, const Object * emitter = NULL);
};

void Corpse::emit(const std::string &event, const Object * emitter) {
	if (event == "collision") {
		if (emitter->classname == "bullet") {
			hp -= emitter->hp;
			if (hp <= 0) 
				emit("death", emitter);
		}
	} else Object::emit(event, emitter);
}


Object* Corpse::clone() const  {
	AnimatedObject *a = new Corpse;
	ResourceManager->initMe(a, "dead-green-tank");
	a->speed = 0;
	a->hp = 10;
	a->ttl = 10;
	a->play("main", true);
	return a;
}

REGISTER_OBJECT("corpse", Corpse, ());
