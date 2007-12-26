
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
#include "zbox.h"
#include "tmx/map.h"
#include "special_owners.h"

BaseObject::BaseObject(const std::string &classname): 
	size(), mass(1), speed(0), ttl(-1), impassability(1), hp(1), max_hp(1), 
 	piercing(false), pierceable(false),
	classname(classname), disable_ai(false), 
	_id(0), _state(), 
	_velocity(), _direction(1,0), 
	_moving_time(0), _idle_time(0), 
	_need_sync(true),
	_dead(false), 
	_position(), _interpolation_progress(1), _z(0), 
	_owners(), _owner_set(), _spawned_by(0) {
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
	_z = other->_z;
}


void BaseObject::serialize(mrt::Serializator &s) const {
	s.add(_id);
	
	s.add(_need_sync);
	
	s.add(_velocity);
	s.add(_direction);
	s.add(_moving_time);	
	s.add(_idle_time);	
	if (_interpolation_progress < 1.0f) {
		s.add(_position + _interpolation_vector * ( 1.0f - _interpolation_progress));
	} else 
		s.add(_position);
	s.add(_z);

	if (!_need_sync)
		return;

	s.add(size);
	s.add(mass);
	s.add(speed);
	s.add(ttl);
	s.add(impassability);
	s.add(hp);
	s.add(max_hp);
	s.add(piercing);
	s.add(pierceable);
	s.add(classname);
	s.add(disable_ai);
	
	s.add(_state);
	
	//s.add(_dead); 
	s.add(_variants);


	s.add(_owners);	
	s.add(_spawned_by);
}

void BaseObject::deserialize(const mrt::Serializator &s) {
	s.get(_id);
	
	s.get(_need_sync);
	
	s.get(_velocity);
	s.get(_direction);
	s.get(_moving_time);	
	s.get(_idle_time);	

	interpolate();
	_position.deserialize(s);
	s.get(_z);
	if (!_need_sync)
		return;

	s.get(size);
	s.get(mass);
	s.get(speed);
	s.get(ttl);
	s.get(impassability);
	s.get(hp);
	s.get(max_hp);
	s.get(piercing);
	s.get(pierceable);
	s.get(classname);
	s.get(disable_ai);
	
	s.get(_state);
	
	//s.get(_dead);
	_dead = false; //mark object as undead only after full deserialization.
	s.get(_variants);
	
	_owners.clear();
	_owner_set.clear();
	int n;
	s.get(n);
	while(n--) {
		int id;
		s.get(id);
		_owners.push_back(id);
		_owner_set.insert(id);
	}
	if (_owners.size() != _owner_set.size()) { 
		std::string o;
		for(std::deque<int>::const_iterator i = _owners.begin(); i != _owners.end(); ++i) 
			o += mrt::formatString("%d,", *i);
		throw_ex(("broken/duplicate owners recv'ed: %s [%u/%u]", o.substr(0, o.size() - 1).c_str(), (unsigned)_owners.size(), (unsigned)_owner_set.size()));
	}
		
	s.get(_spawned_by);
}

const std::string BaseObject::dump() const {
	return mrt::formatString("object '%s', mass: %g, speed: %g, ttl: %g, impassability: %g, hp: %d, piercing: %s, pierceable: %s, z: %d, dead: %s",
		classname.c_str(), mass, speed, ttl, impassability, hp, piercing?"true":"false", pierceable?"true":"false", _z, _dead?"true":"false"
	);
}

BaseObject::~BaseObject() { _dead = true; }

const float BaseObject::getCollisionTime(const v2<float> &dpos, const v2<float> &vel, const float range) const {
	if (vel.is0())
		return -1;

	float r = dpos.length(), v = vel.length(), t = r / v;
	v2<float> d = dpos + vel * t;
	r = d.length();
	if (r <= range)
		return t;
	return -1;
		
/*	//v2<float> dpos = pos - _position;
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
*/
}

void BaseObject::convertToAbsolute(v2<float> &pos, const v2<float> &dpos) {
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

void BaseObject::setZ(const int z0, const bool absolute) {
	if (absolute) {
		_z = z0;
		return;
	}

	int z = z0;
	
	if (z < -1000 || z >= 1000) {
		LOG_WARN(("setZ(%d, !absolute) called. call setZBox to change z-box instead", z));
		z -= ZBox::getBoxBase(z);
	}
	_z = ZBox::getBoxBase(_z) + z; //do not change box;
}

void BaseObject::heal(const int plus) {
	if (hp >= max_hp)
		return;
		
	_need_sync = true;
	hp += plus;
	if (hp >= max_hp)
		hp = max_hp;
	LOG_DEBUG(("%s: got %d hp (heal). result: %d", classname.c_str(), plus, hp));
}

const bool BaseObject::take(const BaseObject *obj, const std::string &type) {
	if (hp < max_hp && obj->classname == "heal" ) {
		heal(obj->hp);
		return true;
	}
	//LOG_WARN(("%s: cannot take %s (%s)", classname.c_str(), obj->classname.c_str(), type.c_str()));
	return false;
}

void BaseObject::disown() {
	_owners.clear();
	_owner_set.clear();
}

void BaseObject::copyOwners(const BaseObject *from) {
	if (this == from)
		return;
	
	_owners = from->_owners;
	_owner_set = from->_owner_set;
	assert(_owners.size() == _owner_set.size());
}


void BaseObject::addOwner(const int oid) {
	if (hasOwner(oid))
		return;
	
	_owners.push_front(oid);
	_owner_set.insert(oid);
	assert(_owners.size() == _owner_set.size());
}

void BaseObject::prependOwner(const int oid) {
	if (hasOwner(oid))
		return;
	
	_owners.push_back(oid);
	_owner_set.insert(oid);
	LOG_DEBUG(("%s[%d] called prependSlot(%d)", classname.c_str(), _id, oid));
	assert(_owners.size() == _owner_set.size());
}

const int BaseObject::_getOwner() const {
	assert(!_owners.empty());
	return *_owners.begin();
}

const bool BaseObject::hasSameOwner(const BaseObject *other, const bool skip_cooperative) const {
	assert(this != other);
	if (hasOwner(other->_id) || other->hasOwner(_id))
		return true;
	
	std::set<int>::const_iterator i = _owner_set.begin(), j = other->_owner_set.begin();
	while(i != _owner_set.end() && j != other->_owner_set.end()) {
		const int l = *i, r = *j;
		if (l == r) {
			if (skip_cooperative && ((l == OWNER_COOPERATIVE && (!piercing && !other->piercing)) || l == OWNER_MAP)) {
				++i; ++j;
				continue;
			}
			return true;
			//LOG_DEBUG(("same owner: %s: %d: %d : %s", classname.c_str(), l, r, other->classname.c_str()));
		}
		
		if (l < r) {
			++i;
		} else {
			++j;
		}
	}
	
	/*
	LOG_DEBUG(("no same owner: %s(%u): %s(%u)", classname.c_str(), _owner_set.size(), other->classname.c_str(), other->_owner_set.size()));
	for(std::set<int>::const_iterator i = _owner_set.begin(); i != _owner_set.end(); ++i) {
		LOG_DEBUG(("%s: %d", classname.c_str(), *i));
	}
	for(std::set<int>::const_iterator i = other->_owner_set.begin(); i != other->_owner_set.end(); ++i) {
		LOG_DEBUG(("%s: %d", other->classname.c_str(), *i));
	}
	*/
	return false;
}

const bool BaseObject::hasOwner(const int oid) const {
	return _owner_set.find(oid) != _owner_set.end();
}

void BaseObject::removeOwner(const int oid) {
	_owner_set.erase(oid);
	for(std::deque<int>::iterator i = _owners.begin(); i != _owners.end(); ) {
		if (*i == oid) {
			i = _owners.erase(i);
		} else ++i;
	}
	assert(_owners.size() == _owner_set.size());
}

void BaseObject::truncateOwners(const int n) {
	assert(0); /*implement me*/
	if ((int)_owners.size() > n) 
		_owners.resize(n);
}


const v2<float> BaseObject::getRelativePosition(const BaseObject *obj) const {
	return Map->distance(this->getCenterPosition(), obj->getCenterPosition());
}

const bool BaseObject::updatePlayerState(const PlayerState &state) {
	bool updated = _state != state;
	if (updated) {
		//LOG_DEBUG(("player %d:%s updated state: %s -> %s", _id, classname.c_str(), _state.dump().c_str(), state.dump().c_str()));
		_state = state;
	}
	return updated;
}

void BaseObject::getInfo(v2<float> &pos, v2<float> &vel) const {
	pos = _position;
	vel = _velocity;
	
	vel.normalize();
	vel *= speed;
}

void BaseObject::updateStateFromVelocity() {
	PlayerState state = _state;
	state.left = (_velocity.x < 0);
	state.right = (_velocity.x > 0);
	state.up = (_velocity.y < 0);
	state.down = (_velocity.y > 0);
}

void BaseObject::interpolate() {
	_interpolation_position_backup = _position;
	_interpolation_progress = 1.0f;
}

void BaseObject::uninterpolate() {
	if (_interpolation_progress >= 1.0f)
		return;
	
	_position += _interpolation_vector * (1.0f - _interpolation_progress);
	_interpolation_position_backup.clear();
}

void BaseObject::getImpassabilityPenalty(const float impassability, float &base, float &base_value, float &penalty) const {
	base = 0;
	base_value = 0;
	penalty = 1;
}

const float BaseObject::getEffectiveImpassability(const float impassability) const {
	if (impassability >= 1.0f)
		return 1.0f;
	
	float base = 0, base_value = 0, penalty = 1.0f;
	getImpassabilityPenalty(impassability, base, base_value, penalty);
	if (base > impassability)
		throw_ex(("invalid impassability penalty returned for %g: base: %g, penalty: %g (base is out of range)", impassability, base, penalty));
	float eim = base_value + (impassability - base) * penalty;
	if (eim < 0)
		eim = 0;
	if (eim > 1) 
		eim = 1;
	return eim;
}

void BaseObject::updateVariants(const Variants &vars, const bool remove_old) {
	_variants.update(vars, remove_old);
}
