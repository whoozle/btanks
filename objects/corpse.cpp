#include "object.h"
#include "resource_manager.h"

class Corpse : public Object {
public:
	Corpse(const int fc) : Object("corpse"), _fire_cycles(fc) {}

	virtual Object * clone() const;
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);
	virtual void tick(const float dt);
	virtual void onSpawn();
private: 
	int _fire_cycles;
};

void Corpse::emit(const std::string &event, BaseObject * emitter) {
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

void Corpse::tick(const float dt) {
	Object::tick(dt);
	if (!getState().size()) {	
		//LOG_DEBUG(("over"));
		emit("death", this);
	}
}

void Corpse::onSpawn() {
	//LOG_DEBUG(("single-pose: play('%s', %s)", _pose.c_str(), _repeat?"true":"false"));
	play("fade-in", false);
	for(int i = 0; i < _fire_cycles; ++i)
		play("burn", false);
	play("fade-out", false);
	play("dead", true);
}


Object* Corpse::clone() const  {
	return new Corpse(*this);
}

REGISTER_OBJECT("corpse", Corpse, (10));
