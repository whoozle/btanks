#include "object.h"
#include "resource_manager.h"

class Item : public Object {
public:
	const std::string type;
	Item(const std::string &classname, const std::string &type = std::string()) : Object(classname), type(type) {
		pierceable = true;
		impassability = 1;
	}
	virtual Object * clone() const;
	virtual void onSpawn();
	virtual void tick(const float dt);
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);
};

void Item::tick(const float dt) {
	Object::tick(dt);
	if (getState().empty()) 
		Object::emit("death", this);
}

void Item::onSpawn() {
	play("main", true);
}

void Item::emit(const std::string &event, BaseObject * emitter) {
	if (event == "collision") {
		if (emitter->classname != "player")
			return;
		
		if (!emitter->take(this, type)) {
			return;
		}

		hp = 0;
		impassability = 0;
		setZ(5); //fly up on the vehicle
		cancelAll();
		play("take", false);
	} else Object::emit(event, emitter);
}


Object* Item::clone() const  {
	return new Item(*this);
}

/*  note that all heal objects have the same classname. this was done to simplify AI search/logic.*/

REGISTER_OBJECT("heal", Item, ("heal"));
REGISTER_OBJECT("megaheal", Item, ("heal"));

REGISTER_OBJECT("guided-missiles-item", Item, ("missiles", "guided"));
REGISTER_OBJECT("dumb-missiles-item", Item, ("missiles", "dumb"));
REGISTER_OBJECT("smoke-missiles-item", Item, ("missiles", "smoke"));

REGISTER_OBJECT("mines-item", Item, ("mines", "mine"));
