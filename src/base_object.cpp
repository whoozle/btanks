#include "base_object.h"
#include "mrt/logger.h"
#include "world.h"

int BaseObject::_last_id;

BaseObject::BaseObject(const std::string &classname)
 : mass(1), speed(1), ttl(-1), impassability(1), hp(1), piercing(false), 
   classname(classname), _id(++_last_id), _direction(1,0,0),  _dead(false), _owner_id(0) {
	//LOG_DEBUG(("allocated id %ld", _id));
}

void BaseObject::serialize(mrt::Serializator &s) const {
	s.add(_id);
	s.add(_owner_id);

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

void BaseObject::deserialize(const mrt::Serializator &s) {
	s.get(_id);
	s.get(_owner_id);

	s.get(mass);
	s.get(speed);
	s.get(ttl);
	s.get(impassability);
	s.get(hp);
	s.get(piercing);
	s.get(classname);
	s.get(_dead);

	size.deserialize(s);
	_velocity.deserialize(s);
	_old_velocity.deserialize(s);
	_direction.deserialize(s);

	_position.deserialize(s);
}

BaseObject::~BaseObject() {}

void BaseObject::getPosition(v3<float> &position) {
	position = _position;
}

const bool BaseObject::isDead() const { return _dead;}


void BaseObject::emit(const std::string &event, const BaseObject * emitter) {
	if (event == "death") {
		_velocity.clear();
		_dead = true;
	} else LOG_WARN(("%s[%d]: unhandled event '%s'", classname.c_str(), _id, event.c_str()));
}

const float BaseObject::getCollisionTime(const v3<float> &dpos, const v3<float> &vel) const {
	//v3<float> dpos = pos - _position;
	float a = vel.x * vel.x + vel.y * vel.y;
	if (a == 0)
		return -1;
	//LOG_DEBUG(("a = %f", a));
	float b = 2 * (vel.x * dpos.x + vel.y * dpos.y) ;
	float r = ((size.x + size.y) / 2);
	float c = dpos.x * dpos.x + dpos.y * dpos.y - r * r;
	//LOG_DEBUG(("dpos: %f %f", dpos.x, dpos.y));
	//LOG_DEBUG(("b = %f, c = %f, r = %f", b, c, r));
	
	if (b/a > 0 && c/a > 0) //both t1,t2 < 0
		return -2;
	
	float d = b * b - 4 * a * c;
	if (d < 0) 
		return -3; //no solution

	d = sqrt(d);
	
	float t1 = (-b + d) / 2 / a;
	if (t1 > 0) 
		return t1;
		
	float t2 = (-b - d) / 2 / a;
	if (t2 > 0)
		return t2;
	
	return -4;
}

