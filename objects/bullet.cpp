#include "object.h"
#include "resource_manager.h"

class Bullet : public Object {
public:
	Bullet(const std::string &type) : Object("bullet"), _type(type) {}
	virtual void calculate(const float dt);
	virtual Object * clone() const;
	virtual void onSpawn();
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_type);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_type);
	}

private: 
	std::string _type;
};


void Bullet::calculate(const float dt) {}

void Bullet::onSpawn() {
	play("move", true);
	_velocity.normalize();
	
	int dir = _velocity.getDirection8();
	if (dir) {
		setDirection(dir - 1);
	}
	_velocity.quantize8();
}

void Bullet::emit(const std::string &event, BaseObject * emitter) {
	if (event == "collision" || event == "death") {
		v3<float> dpos;
		if (emitter)
			dpos = getRelativePos(emitter) / 2;
		
		dpos.z = 1;
		if (_type == "regular") {
			spawn("explosion", "explosion", dpos, v3<float>(0,0,0));
		} else if (_type == "dirt") {
			spawn("dirt", "dirt", dpos, v3<float>(0,0,0));
		}
		Object::emit("death", emitter);
	} else Object::emit(event, emitter);
}


Object* Bullet::clone() const  {
	Object *a = new Bullet(*this);
	a->setDirection(getDirection());
	return a;
}

REGISTER_OBJECT("bullet", Bullet, ("regular"));
REGISTER_OBJECT("dirt-bullet", Bullet, ("dirt"));
