#include "object.h"
#include "resource_manager.h"

class Paratrooper : public Object {
public:
	Paratrooper(const std::string &classname, const std::string &spawn_object, const std::string &spawn_animation) : 
		Object(classname), _spawn_object(spawn_object), _spawn_animation(spawn_animation) { 
	}
	virtual void tick(const float dt);
	virtual Object * clone() const;
	virtual void onSpawn();
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);
	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_spawn_object);
		s.add(_spawn_animation);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_spawn_object);
		s.get(_spawn_animation);
	}
private:
	std::string _spawn_object, _spawn_animation;
};


void Paratrooper::tick(const float dt) {
	Object::tick(dt);
	if (getState().empty()) {	
		//LOG_DEBUG(("over"));
		spawn(_spawn_object, _spawn_animation);
		emit("death", this);
	}
}

void Paratrooper::onSpawn() {
	setDirection(0);
	play("main", false);
}

void Paratrooper::emit(const std::string &event, BaseObject * emitter) {
	Object::emit(event, emitter);
}


Object* Paratrooper::clone() const  {
	Object *a = new Paratrooper(*this);
	return a;
}

REGISTER_OBJECT("paratrooper-kamikaze", Paratrooper, ("paratrooper", "kamikaze", "kamikaze"));
