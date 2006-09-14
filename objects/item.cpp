#include "object.h"
#include "resource_manager.h"

class Item : public Object {
public:
	Item(const std::string &classname) : Object(classname) {
		pierceable = true;
	}
	virtual Object * clone() const;
	virtual void onSpawn();
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);
};

void Item::onSpawn() {
	play("main", true);
}

void Item::emit(const std::string &event, BaseObject * emitter) {
	if (event == "collision") {
		if (emitter->classname != "player") 
			return;
		if (classname == "heal") {
			emitter->heal(hp);
		} else LOG_WARN(("item '%s' was not implemented", classname.c_str()));
		Object::emit("death", emitter);
	} else Object::emit(event, emitter);
}


Object* Item::clone() const  {
	return new Item(*this);
}

/*  note that all heal objects have the same classname. this was done to simplify AI search/logic.*/

REGISTER_OBJECT("heal", Item, ("heal"));
REGISTER_OBJECT("megaheal", Item, ("heal"));
