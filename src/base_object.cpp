#include "base_object.h"
#include "mrt/logger.h"
#include "world.h"

BaseObject::BaseObject(const std::string &classname)
 : mass(1), speed(0), ttl(-1), impassability(1), hp(1), max_hp(1), piercing(false), pierceable(false),
   classname(classname), _id(0), _follow(0), _direction(1,0,0), _distance(0), _moving_time(0), _idle_time(0), _dead(false), _owner_id(0) {
	//LOG_DEBUG(("allocated id %ld", _id));
}

void BaseObject::inheritParameters(const BaseObject *other) {
	hp = other->hp;
	mass = other->mass;
	speed = other->speed;
	ttl = other->ttl;
	impassability = other->impassability;
	piercing = other->piercing;
	size = other->size;
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
	_direction.deserialize(s);

	_position.deserialize(s);
}

const std::string BaseObject::dump() const {
	return mrt::formatString("object '%s', mass: %g, speed: %g, ttl: %g, impassability: %g, hp: %d, piercing: %s, pierceable: %s, dead: %s",
		classname.c_str(), mass, speed, ttl, impassability, hp, piercing?"true":"false", pierceable?"true":"false", _dead?"true":"false"
	);
}

BaseObject::~BaseObject() {}

void BaseObject::emit(const std::string &event, BaseObject * emitter) {
	if (event == "death") {
		_velocity.clear();
		_dead = true;
	} else LOG_WARN(("%s[%d]: unhandled event '%s'", classname.c_str(), _id, event.c_str()));
}

const float BaseObject::getCollisionTime(const v3<float> &dpos, const v3<float> &vel, const float r) const {
	//v3<float> dpos = pos - _position;
	float a = vel.x * vel.x + vel.y * vel.y;
	if (a == 0)
		return -1;
	//LOG_DEBUG(("a = %g", a));
	float b = 2 * (vel.x * dpos.x + vel.y * dpos.y) ;
	float c = dpos.x * dpos.x + dpos.y * dpos.y - r * r;
	//LOG_DEBUG(("dpos: %g %g", dpos.x, dpos.y));
	//LOG_DEBUG(("b = %g, c = %g, r = %g", b, c, r));
	
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

void BaseObject::convertToAbsolute(v3<float> &pos, const v3<float> &dpos) {
	pos = _position;
	pos += dpos;
}

void BaseObject::calculate(const float dt) {
	_velocity.clear();
		
	if (_state.left) _velocity.x -= 1;
	if (_state.right) _velocity.x += 1;
	if (_state.up) _velocity.y -= 1;
	if (_state.down) _velocity.y += 1;
	
	_velocity.normalize();
}

/*

void BaseObject::pretick() {
	if (_stateless)
		return;

	state2velocity();
}

void BaseObject::posttick() {
	//AI player will be easier to implement if operating directly with velocity
	if (!_stateless || _velocity.is0()) 
		return;
	
	//LOG_DEBUG(("class: %s", classname.c_str()));
	_velocity.normalize();
		
	v3<float>::quantize(_velocity.x);	
	v3<float>::quantize(_velocity.y);
	//LOG_DEBUG(("%s: _velocity: %g %g", classname.c_str(), _velocity.x, _velocity.y));
		
	_state.left = _velocity.x == -1;
	_state.right = _velocity.x == 1;
	_state.up = _velocity.y == -1;
	_state.down = _velocity.y == 1;

	state2velocity();
}

void BaseObject::state2velocity() {
	_velocity.clear();
		
	if (_state.left) _velocity.x -= 1;
	if (_state.right) _velocity.x += 1;
	if (_state.up) _velocity.y -= 1;
	if (_state.down) _velocity.y += 1;
	
	_velocity.normalize();
}
*/
void BaseObject::follow(const BaseObject *obj, const GroupType mode) {
	_follow = obj->_id;
	if (mode == Centered) {
		_follow_position = (obj->size - size) / 2;
		LOG_DEBUG(("follow: %g %g", _follow_position.x, _follow_position.y));
	}
}

void BaseObject::follow(const int id) {
	_follow = id;
}

void BaseObject::addDamage(BaseObject *from) {
	if (!from->piercing)
		return;
	hp -= from->hp;	
	LOG_DEBUG(("%s: received %d hp of damage from %s. hp = %d", classname.c_str(), from->hp, from->classname.c_str(), hp));
	if (hp <= 0) 
		emit("death", from);
}

void BaseObject::heal(const int n) {	
	hp += n;
	if (hp >= max_hp)
		hp = max_hp;
	LOG_DEBUG(("%s: got %d hp (heal). result: %d", classname.c_str(), n, hp));
}

void BaseObject::setZ(const float z) {
	_position.z = z;
}

