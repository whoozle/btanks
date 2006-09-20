#include "object.h"
#include "resource_manager.h"
#include "game.h"

#include <set>

class Explosion : public Object {
public:
	Explosion(const std::string &classname) : Object(classname) { impassability = 0; }
	virtual void tick(const float dt);
	virtual Object * clone() const;
	virtual void onSpawn();
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);
private:
	std::set<int> _damaged_objects;
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
	if (classname == "nuclear-explosion") 
		Game->shake(1, 4);
}

void Explosion::emit(const std::string &event, BaseObject * emitter) {
	if (event == "collision") {
		if (classname != "nuclear-explosion" || emitter->pierceable)
			return;
		//nuke damage.
		const int id = emitter->getID();
		
		if (_damaged_objects.find(id) != _damaged_objects.end())
			return; //damage was already added for this object.
		
		_damaged_objects.insert(id);
		
		const bool p = piercing;
		piercing = true;
		emitter->addDamage(this);
		piercing = p;
		
	} else Object::emit(event, emitter);
}


Object* Explosion::clone() const  {
	Object *a = new Explosion(*this);
	return a;
}

REGISTER_OBJECT("explosion", Explosion, ("explosion"));
REGISTER_OBJECT("nuclear-explosion", Explosion, ("nuclear-explosion"));
