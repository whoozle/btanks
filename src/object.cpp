#include "object.h"
#include "mrt/logger.h"
#include "world.h"

int Object::_last_id;

Object::Object(const std::string &classname)
 : mass(1), speed(1), ttl(-1), impassability(1), hp(1), piercing(false), 
   classname(classname), _id(++_last_id), _direction(1,0,0),  _dead(false), _owner(NULL) {
	//LOG_DEBUG(("allocated id %ld", _id));
}

void Object::serialize(mrt::Serializator &s) const {
	s.add(_id);

	s.add(mass);
	s.add(speed);
	s.add(ttl);
	s.add(impassability);
	s.add(hp);
	s.add(piercing);
	s.add(classname);
	s.add(_dead);

	size.serialize(s);
	_velocity.serialize(s);
	_old_velocity.serialize(s);
	_direction.serialize(s);

	_position.serialize(s);
}

void Object::deserialize(const mrt::Serializator &s) {}

Object::~Object() {}

void Object::getPosition(v3<float> &position) {
	position = _position;
}

const bool Object::isDead() const { return _dead;}


void Object::emit(const std::string &event, const Object * emitter) {
	if (event == "death") {
		_velocity.clear();
		_dead = true;
	} else LOG_WARN(("unhandled event '%s'", event.c_str()));
}

const Object* Object::spawn(const std::string &classname, const std::string &animation, const v3<float> &dpos, const v3<float> &vel) {
	return World->spawn(this, classname, animation, dpos, vel);
}

const bool Object::getNearest(const std::string &classname, v3<float> &position, v3<float> &velocity) const {
	return World->getNearest(this, classname, position, velocity);
}

const float Object::getCollisionTime(const v3<float> &pos, const v3<float> &vel) const {
	v3<float> dpos = pos - _position;
	float a = vel.x * vel.x + vel.y * vel.y;
	if (a == 0)
		return -1;
	
	float b = 2 * (vel.x * dpos.x + vel.y * dpos.y) ;
	float r = ((size.x + size.y) / 2);
	float c = dpos.x * dpos.x + dpos.y * dpos.y - r*r;
	
	if (b/a > 0 && c/a > 0) //both times < 0
		return -1;
	
	float d = b * b - 4 * a * c;
	if (d < 0) 
		return -1; //no solution

	d = sqrt(d);
	
	float t1 = (-b + d) / 2 / a;
	if (t1 > 0) 
		return t1;
		
	float t2 = (-b - d) / 2 / a;
	if (t2 > 0)
		return t2;
	
	return -1;
}

