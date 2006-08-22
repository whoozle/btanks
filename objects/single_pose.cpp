#include "object.h"
#include "resource_manager.h"

class SinglePose : public Object {
public:
	SinglePose() : Object("single-pose", true) {}
	virtual Object * clone(const std::string &opt) const;
	virtual void emit(const std::string &event, const BaseObject * emitter = NULL);
};

void SinglePose::emit(const std::string &event, const BaseObject * emitter) {
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


Object* SinglePose::clone(const std::string &opt) const  {
	Object *a = new SinglePose(*this);
	ResourceManager->initMe(a, opt);

	a->play("main", true);
	return a;
}

REGISTER_OBJECT("single-pose", SinglePose, ());
REGISTER_OBJECT("corpse", SinglePose, ());
