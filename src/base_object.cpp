
/* Battle Tanks Game
 * Copyright (C) 2006-2007 Battle Tanks team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "base_object.h"
#include "mrt/logger.h"
#include "world.h"

BaseObject::BaseObject(const std::string &classname): 
	size(), mass(1), speed(0), ttl(-1), impassability(1), hp(1), max_hp(1), 
 	piercing(false), pierceable(false),
	classname(classname), 
	_id(0), _follow(0), _follow_position(), 
	_state(), 
	_velocity(), _direction(1,0,0), _velocity_fadeout(), 
	_moving_time(0), _idle_time(0), 
	need_sync(false),
	_dead(false), 
	_position(),
	_owners(), _spawned_by(0) {
	//LOG_DEBUG(("allocated id %ld", _id));
}

void BaseObject::inheritParameters(const BaseObject *other) {
	mass = other->mass;
	speed = other->speed;
	ttl = other->ttl;
	impassability = other->impassability;
	hp = other->hp;
	max_hp = other->max_hp;
	piercing = other->piercing;
	pierceable = other->pierceable;
	size = other->size;
	_position.z = other->_position.z;
}


void BaseObject::serialize(mrt::Serializator &s) const {
	s.add(_id);
	
	size.serialize(s);
	s.add(mass);
	s.add(speed);
	s.add(ttl);
	s.add(impassability);
	s.add(hp);
	s.add(max_hp);
	s.add(piercing);
	s.add(pierceable);
	s.add(classname);
	
	s.add(_follow);
	_follow_position.serialize(s);
	_velocity.serialize(s);
	_direction.serialize(s);
	_velocity_fadeout.serialize(s);
	s.add(_moving_time);	
	s.add(_idle_time);	

	s.add(_dead);
	_position.serialize(s);
	
	int n = _owners.size();
	s.add(n);
	for(std::deque<int>::const_iterator i = _owners.begin(); i != _owners.end(); ++i) 
		s.add(*i);
		
	s.add(_spawned_by);
}

void BaseObject::deserialize(const mrt::Serializator &s) {
	s.get(_id);
	
	size.deserialize(s);
	s.get(mass);
	s.get(speed);
	s.get(ttl);
	s.get(impassability);
	s.get(hp);
	s.get(max_hp);
	s.get(piercing);
	s.get(pierceable);
	s.get(classname);
	
	s.get(_follow);
	_follow_position.deserialize(s);
	_velocity.deserialize(s);
	_direction.deserialize(s);
	_velocity_fadeout.deserialize(s);
	s.get(_moving_time);	
	s.get(_idle_time);	

	s.get(_dead);
	_position.deserialize(s);
	
	_owners.clear();
	int n;
	s.get(n);
	while(n--) {
		int id;
		s.get(id);
		_owners.push_back(id);
	}
	
	s.get(_spawned_by);
}

const std::string BaseObject::dump() const {
	return mrt::formatString("object '%s', mass: %g, speed: %g, ttl: %g, impassability: %g, hp: %d, piercing: %s, pierceable: %s, z: %g, dead: %s",
		classname.c_str(), mass, speed, ttl, impassability, hp, piercing?"true":"false", pierceable?"true":"false", _position.z, _dead?"true":"false"
	);
}

BaseObject::~BaseObject() { _dead = true; }

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

/*
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
		//LOG_DEBUG(("follow: %g %g", _follow_position.x, _follow_position.y));
	}
	_position.x = obj->_position.x;
	_position.y = obj->_position.y;
}

void BaseObject::follow(const int id) {
	_follow = id;
}




void BaseObject::setZ(const float z) {
	_position.z = z;
}

const bool BaseObject::take(const BaseObject *obj, const std::string &type) {
	if (obj->classname == "heal") {
		if (hp >= max_hp)
			return false;
		
		need_sync = true;
		hp += obj->hp;
		if (hp >= max_hp)
			hp = max_hp;
		LOG_DEBUG(("%s: got %d hp (heal). result: %d", classname.c_str(), obj->hp, hp));	
		return true;
	}
	//LOG_WARN(("%s: cannot take %s (%s)", classname.c_str(), obj->classname.c_str(), type.c_str()));
	return false;
}

void BaseObject::disown() {
	_owners.clear();
}

void BaseObject::addOwner(const int oid) {
	if (hasOwner(oid))
		return;
	
	_owners.push_front(oid);
}

const int BaseObject::_getOwner() const {
	return *_owners.begin();
}

const bool BaseObject::hasSameOwner(const BaseObject *other) const {
	for(std::deque<int>::const_iterator j = other->_owners.begin(); j != other->_owners.end(); ++j) 
		if (*j == _id)
			return true;

	for(std::deque<int>::const_iterator i = _owners.begin(); i != _owners.end(); ++i) {
		if (*i == other->_id)
			return true;
		
		for(std::deque<int>::const_iterator j = other->_owners.begin(); j != other->_owners.end(); ++j) 
			if (*i == *j)
				return true;
	}
	return false;
}

const bool BaseObject::hasOwner(const int oid) const {
	for(std::deque<int>::const_iterator i = _owners.begin(); i != _owners.end(); ++i)
		if (*i == oid)
			return true;
	return false;
}

void BaseObject::removeOwner(const int oid) {
	for(std::deque<int>::iterator i = _owners.begin(); i != _owners.end(); ) {
		if (*i == oid) {
			i = _owners.erase(i);
		} else ++i;
	}
}

void BaseObject::truncateOwners(const int n) {
	if ((int)_owners.size() > n) 
		_owners.resize(n);
}


const v3<float> BaseObject::getRelativePosition(const BaseObject *obj) const {
	return obj->_position - _position - size / 2 + obj->size / 2;
}

const bool BaseObject::updatePlayerState(const PlayerState &state) {
	bool updated = _state != state;
	if (updated) {
		LOG_DEBUG(("player %d:%s updated state: %s -> %s", _id, classname.c_str(), _state.dump().c_str(), state.dump().c_str()));
		_state = state;
		need_sync = true;
	}
	return updated;
}

void BaseObject::getInfo(v3<float> &pos, v3<float> &vel) const {
	pos = _position;
	vel = _velocity;
	
	vel.normalize();
}

void BaseObject::updateStateFromVelocity() {
	_state.left = (_velocity.x < 0);
	_state.right = (_velocity.x > 0);
	_state.up = (_velocity.y < 0);
	_state.down = (_velocity.y > 0);
}
