#include "object.h"
#include "resource_manager.h"

class Mine : public Object {
public:
	Mine() : Object("mine") {
		piercing = true;
	}
	virtual Object * clone() const;
	virtual void onSpawn();
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);
};

void Mine::onSpawn() {
	play("main", true);
}

void Mine::emit(const std::string &event, BaseObject * emitter) {
	if (event == "collision") {
		spawn("explosion", "explosion", v3<float>(0,0,1), v3<float>(0,0,0));
		Object::emit("death", emitter);
	} else Object::emit(event, emitter);
}


Object* Mine::clone() const  {
	return new Mine(*this);
}

REGISTER_OBJECT("regular-mine", Mine, ());
