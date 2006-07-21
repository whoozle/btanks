#include "animated_object.h"
#include "resource_manager.h"

class Corpse : public AnimatedObject {
public:
	Corpse() : AnimatedObject("corpse") {}
	virtual Object * clone(const std::string &opt) const;
	virtual void emit(const std::string &event, const Object * emitter = NULL);
};

void Corpse::emit(const std::string &event, const Object * emitter) {
	if (event == "collision") {
		if (emitter->classname == "bullet") {
			hp -= emitter->hp;
			if (hp <= 0) {
				emit("death", emitter);
				spawn("explosion", "explosion", v3<float>(0,0,1), v3<float>());
			}
		}
	} else Object::emit(event, emitter);
}


Object* Corpse::clone(const std::string &opt) const  {
	AnimatedObject *a = new Corpse;
	ResourceManager->initMe(a, opt);
	a->speed = 0;
	a->hp = 10;
	a->ttl = 60;
	a->play("main", true);
	return a;
}

REGISTER_OBJECT("corpse", Corpse, ());
