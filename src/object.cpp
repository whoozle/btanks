
/* Battle Tanks Game
 * Copyright (C) 2006 Battle Tanks team
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

#include "object.h"
#include "config.h"
#include "sdlx/surface.h"
#include "sdlx/c_map.h"
#include "mrt/exception.h"
#include "mrt/logger.h"
#include "animation_model.h"
#include "resource_manager.h"
#include "world.h"
#include "math/unary.h"
#include "sound/mixer.h"

Object::Event::Event() : name(), repeat(false), sound(), played(false) {}

Object::Event::Event(const std::string name, const bool repeat, const std::string &sound): 
	name(name), repeat(repeat), sound(sound), played(false) {}
	
void Object::Event::serialize(mrt::Serializator &s) const {
	s.add(name);
	s.add(repeat);
}
void Object::Event::deserialize(const mrt::Serializator &s) {
	s.get(name);
	s.get(repeat);
}

Object * Object::clone() const {
	assert(0);
	return NULL;
}

Object::Object(const std::string &classname) : 
	BaseObject(classname), 
	registered_name(), animation(), fadeout_time(0),  
	_model(0), _model_name(), _surface(0), _cmap(0), _surface_name(), 
	_events(), _effects(), 
	_tw(0), _th(0), _direction_idx(0), _directions_n(8), _pos(0), 
	_way(), _next_target(), _next_target_rel(), 
	_rotation_time(0), 
	_dst_direction(0), 
	
	_group()
	 {}

void Object::init(const std::string &model, const std::string &surface, const int tile_w, const int tile_h) {
	_events.clear();

	_model = ResourceManager->getAnimationModel(model);
	_model_name = model;

	_surface = ResourceManager->getSurface(surface);
	_cmap = ResourceManager->getCollisionMap(surface);
	_surface_name = surface;

	size.x = _tw = tile_w; size.y = _th = tile_h;
	_direction_idx = 0;
	_pos = 0;
}

void Object::init(const Object *a) {
	_model = a->_model;
	_model_name = a->_model_name;
	_surface = a->_surface;
	_cmap = a->_cmap;
	_surface_name = a->_surface_name;
	_events = a->_events;
	_tw = a->_tw;
	_th = a->_th;
	_pos = a->_pos;
	
	size = a->size;
}


Object* Object::spawn(const std::string &classname, const std::string &animation, const v3<float> &dpos, const v3<float> &vel) {
	return World->spawn(this, classname, animation, dpos, vel);
}

const bool Object::getNearest(const std::string &classname, v3<float> &position, v3<float> &velocity, Way * way) const {
	return World->getNearest(this, classname, position, velocity, way);
}

void Object::setDirection(const int dir) {
	if (dir >= _directions_n)
		LOG_WARN(("%s:%s setDirection(%d) called on object with %d directions", classname.c_str(), animation.c_str(), dir, _directions_n));
	if (dir >= 0)
		_direction_idx = dir;
}

const int Object::getDirection() const {
	return _direction_idx;
}

const int Object::getDirectionsNumber() const {
	return _directions_n;
}

void Object::setDirectionsNumber(const int dirs) {
	if (dirs >= 0) 
		_directions_n = dirs;
}

void Object::quantizeVelocity() {
	int dir;
	_velocity.normalize();
	if (_directions_n == 8) {
		_velocity.quantize8();
		dir = _velocity.getDirection8();
	} else if (_directions_n == 16) {
		_velocity.quantize16();
		dir = _velocity.getDirection16();	
	} else throw_ex(("%s:%s cannot handle %d directions", classname.c_str(), animation.c_str(), _directions_n));
	setDirection(dir - 1);
}


void Object::play(const std::string &id, const bool repeat) {
	if (_events.empty())
		_pos = 0;
	const Pose *pose = _model->getPose(id);
	if (pose == NULL) {
		LOG_WARN(("animation model %s does not have pose %s", _model_name.c_str(), id.c_str()));
		return;
	}

	_events.push_back(Event(id, repeat, pose->sound));
}

void Object::playNow(const std::string &id) {
	const Pose *pose = _model->getPose(id);
	if (pose == NULL) {
		LOG_WARN(("animation model %s does not have pose %s", _model_name.c_str(), id.c_str()));
		return;
	}
	_pos = 0;
	_events.push_front(Event(id, false, pose->sound));
}

void Object::cancel() {
	if (_events.empty()) 
		return;

	Mixer->cancelSample(this, _events.front().sound);

	_events.pop_front();
	_pos = 0;
}

void Object::cancelRepeatable() {
	for (EventQueue::iterator i = _events.begin(); i != _events.end();) {
		if (i->repeat) {
			if (i == _events.begin())
				_pos = 0;
			Mixer->cancelSample(this, i->sound);
			i = _events.erase(i);
		} 
		else ++i;
	}
}


void Object::cancelAll() {
	Mixer->cancelAll(this);
	_events.clear();
	_pos = 0;
}



void Object::tick(const float dt) {
	for(EffectMap::iterator ei = _effects.begin(); ei != _effects.end(); ) {
		if (ei->second < 0) {
			++ei;
			continue;
		}
		ei->second -= dt;
		if (ei->second <= 0) {
			_effects.erase(ei++);
			continue;
		}
		if (ei->first == "stunned") {
			_velocity.clear();
		}
		++ei;
	}

	if (_events.empty()) 
		return;
	
	Event & event = _events.front();
	//LOG_DEBUG(("%p: event: %s, pos = %f", (void *)this, event.name.c_str(), _pos));
	const Pose * pose = _model->getPose(event.name);
	
	if (pose == NULL) {
		LOG_WARN(("animation model %s does not have pose %s", _model_name.c_str(), event.name.c_str()));
		cancel();
		return;
	}
	
	if (pose->z > -1000) {
		setZ(pose->z);
	}
	
	if (!event.played) {
		event.played = true;
		Mixer->playSample(this, event.sound, event.repeat);
	}
	
	_pos += dt * pose->speed;
	int n = pose->frames.size();
	if (n == 0) {
		LOG_WARN(("animation model %s, pose %s doesnt contain any frames", _model_name.c_str(), event.name.c_str()));
		return;
	}
		
	int cycles = (int)(_pos / n);
	//LOG_DEBUG(("%s: _pos: %f, cycles: %d", classname.c_str(), _pos, cycles));
	_pos -= cycles * n;
	while((int)_pos >= n) {
		_pos -= n;
		++cycles;
	}
	
	if (cycles) {
		if (!event.repeat)
			cancel();
	} 
}

const bool Object::getRenderRect(sdlx::Rect &src) const {
	if (_events.empty()) {
		if (!isDead())
			LOG_WARN(("%s: no animation played. latest position: %g", classname.c_str(), _pos));
		return false;
	}

	const Pose * pose = _model->getPose(_events.front().name);
	if (pose == NULL) {
		LOG_WARN(("%s: pose '%s' is not supported", classname.c_str(), _events.front().name.c_str()));
		return false;
	}
	
	int frame = (int)_pos;
	int n = (int)pose->frames.size();
	if (n == 0) {
		LOG_WARN(("%s: pose '%s' doesnt have any frames", classname.c_str(), _events.front().name.c_str()));
		return false;
	}
	
	//this stuff need to be fixed, but I still cannot find cause for overflowing frame
	if (frame >= n)
		frame = n - 1;
	
	if (frame < 0 || frame >= n) {
		LOG_WARN(("%s: event '%s' frame %d is out of range (position: %g).", classname.c_str(), _events.front().name.c_str(), frame, _pos));
		return false;	
	}

	frame = pose->frames[frame];
	
	
	const_cast<Object*>(this)->checkSurface();
	
	if (frame * _th >= _surface->getHeight()) {
		LOG_WARN(("%s: event '%s' tile row %d is out of range.", classname.c_str(), _events.front().name.c_str(), frame));
		return false;
	}

	src.x = _direction_idx * _tw;
	src.y = frame * _th;
	src.w = _tw;
	src.h = _th;
	return true;
}

void Object::render(sdlx::Surface &surface, const int x, const int y) {
	sdlx::Rect src;
	if (!getRenderRect(src))
		return;

	int alpha = 0;
	if (fadeout_time > 0 && ttl > 0 && ttl < fadeout_time) 
		alpha = (int)(255 * (fadeout_time - ttl) / fadeout_time);
	//LOG_DEBUG(("alpha = %d", alpha));
	checkSurface();
	
	if (alpha == 0) {
		surface.copyFrom(*_surface, src, x, y);
		return;
	} 
	
	//fade out feature.
	sdlx::Surface blended; 
	blended.createRGB(_tw, _th, 32, 0);
	
	const_cast<sdlx::Surface *>(_surface)->setAlpha(0,0);
	blended.copyFrom(*_surface, src);
	const_cast<sdlx::Surface *>(_surface)->setAlpha(0);

	blended.convertAlpha();

	const int w = blended.getWidth();
	const int h = blended.getHeight();
	
	alpha = 255 - alpha;

	blended.lock();
	//optimize it.
	
	for(int py = 0; py < h; ++py) {
		for(int px = 0; px < w; ++px) {
			Uint8 r, g, b, a;
			blended.getRGBA(blended.getPixel(px, py), r, g, b, a);
			a = (((int)a) * alpha) / 255;
			blended.putPixel(px, py, blended.mapRGBA(r, g, b, a));
		}
	}
	blended.unlock();

	surface.copyFrom(blended, x, y);
}

const bool Object::collides(const Object *other, const int x, const int y, const bool hidden_by_other) const {
	sdlx::Rect src, other_src;
	if (!getRenderRect(src)) 
		return false;
	if (!other->getRenderRect(other_src)) 
		return false;
/*	LOG_DEBUG(("::collide(%s:%d,%d,%d,%d, %s:%d,%d,%d,%d)", 
		classname.c_str(),
		src.x, src.y, src.w, src.h, 
		other->classname.c_str(),
		other_src.x, other_src.y, other_src.w, other_src.h
		));
*/
	return _cmap->collides(src, other->_cmap, other_src, x, y, hidden_by_other);
}

const bool Object::collides(const sdlx::CollisionMap *other, const int x, const int y, const bool hidden_by_other) const {
	sdlx::Rect src;
	if (!getRenderRect(src)) 
		return false;
	return _cmap->collides(src, other, sdlx::Rect(), x, y, hidden_by_other);
}

void Object::serialize(mrt::Serializator &s) const {
	BaseObject::serialize(s);
	
	s.add(animation);
	s.add(fadeout_time);
	
	int en = _events.size();
	s.add(en);
	{
		for(EventQueue::const_iterator i = _events.begin(); i != _events.end(); ++i) {
			i->serialize(s);
		}
	}

	en = _effects.size();
	s.add(en);
	{
		for(EffectMap::const_iterator i = _effects.begin(); i != _effects.end(); ++i) {
			s.add(i->first);
			s.add(i->second);
		}
	}

	
	s.add(_model_name);
	s.add(_surface_name);
	s.add(_tw);
	s.add(_th);
	s.add(_direction_idx);
	s.add(_directions_n);
	s.add(_pos);

	//add support for waypoints here. AI cannot serialize now.
	en = _way.size();
	s.add(en);
	for(Way::const_iterator i = _way.begin(); i != _way.end(); ++i) 
		i->serialize(s);
	_next_target.serialize(s);
	_next_target_rel.serialize(s);
	
	s.add(_rotation_time);
	s.add(_dst_direction);

	//Group	
	en = _group.size();
	s.add(en);
	for(Group::const_iterator i = _group.begin(); i != _group.end(); ++i) {
		s.add(i->first);
		s.add(i->second);
	}
}

void Object::deserialize(const mrt::Serializator &s) {
	BaseObject::deserialize(s);

	s.get(animation);
	s.get(fadeout_time);

	_events.clear();
	int en;
	s.get(en);
	while(en--) {
		Event e;
		e.deserialize(s);
		//LOG_DEBUG(("event: %s, repeat: %s", e.name.c_str(), e.repeat?"true":"false"));
		_events.push_back(e);
	}

	_effects.clear();
	s.get(en);
	while(en--) {
		std::string name;
		float duration;
		s.get(name);
		s.get(duration);
		_effects[name] = duration;
	}
	
	s.get(_model_name);
	s.get(_surface_name);
	s.get(_tw);
	s.get(_th);
	s.get(_direction_idx);
	s.get(_directions_n);
	s.get(_pos);

	s.get(en);
	_way.clear();
	while(en--) {
		v3<int> wp;
		wp.deserialize(s);
		_way.push_back(wp);
	}
	_next_target.deserialize(s);
	_next_target_rel.deserialize(s);


	s.get(_rotation_time);
	s.get(_dst_direction);

	_group.clear();
	s.get(en);
	while(en--) {
		std::string name;
		int id;
		s.get(name);
		s.get(id);
		_group[name] = id;
	}
	//additional initialization
	_model = ResourceManager->getAnimationModel(_model_name);
	_surface = ResourceManager->getSurface(_surface_name);
	_cmap = ResourceManager->getCollisionMap(_surface_name);
}

void Object::emit(const std::string &event, BaseObject * emitter) {
	if (event == "death") {
		for(Group::iterator i = _group.begin(); i != _group.end(); ++i) {
			Object * o = World->getObjectByID(i->second);
			if (o == NULL)
				continue;
			o->emit(event, emitter);
		}
		_group.clear();
	}
	BaseObject::emit(event, emitter);
}

void Object::setWay(const Way & way) {
	_way = way;
	_next_target.clear();
	if (!way.empty()) 
		LOG_DEBUG(("%d:%s:%s set %d pending waypoints", getID(), classname.c_str(), animation.c_str(), _way.size()));
}

void Object::calculateWayVelocity() {
	v3<float> position = getPosition();
	sdlx::Rect me((int)position.x, (int)position.y, (int)size.x, (int)size.y);

	while (!_way.empty()) {
		_velocity.clear();
		
		if (_next_target.is0()) {
			_next_target = _way.begin()->convert<float>();
			v3<float> rel = _next_target - position;
			_way.pop_front();
			
			sdlx::Rect wp_rect((int)_next_target.x, (int)_next_target.y, (int)size.x, (int)size.y);
			
			if (me.intersects(wp_rect)) {
				_next_target.clear();
				_velocity.clear();
				continue;
			}
						
			/*if (!_next_target_rel.is0() && (rel.x == 0 || rel.x * _next_target_rel.x <= 0) && (rel.y == 0 || rel.y * _next_target_rel.y <= 0)) {
				LOG_DEBUG(("skipped waypoint behind objects' back %g:%g (old %g:%g", rel.x, rel.y, _next_target_rel.x, _next_target_rel.y ));
				_next_target.clear();
				continue;
			}
			*/
			
			if (rel.quick_length() < 4) {
				_next_target.clear();
				_velocity.clear();
				continue;
			}
			_next_target_rel = rel;
			//LOG_DEBUG(("waypoints: %d", _way.size()));
		}
		//LOG_DEBUG(("%d:%s:%s next waypoint: %g %g, relative: %g %g", 
		//	getID(), classname.c_str(), animation.c_str(), _next_target.x, _next_target.y, _next_target_rel.x, _next_target_rel.y));
		
		_velocity = _next_target - position;
		if ((_next_target_rel.x != 0 && _velocity.x * _next_target_rel.x <= 0) || (math::abs(_velocity.x) < 1))
			_velocity.x = 0;
		if ((_next_target_rel.y != 0 && _velocity.y * _next_target_rel.y <= 0) || (math::abs(_velocity.y) < 1))
			_velocity.y = 0;
		
		if (_velocity.is0()) {
			//wiping out way point and restart
			_next_target.clear();
		} else break;
	}
	_velocity.normalize();
	//LOG_DEBUG(("%d: velocity: %g %g", getID(), _velocity.x, _velocity.y));
}


const bool Object::isDriven() const {
	return !_way.empty();
}

void Object::setup(const std::string &a) {
	ResourceManager->initMe(this, a);
	animation = a;
	_events.clear();
}

void Object::onSpawn() {
	throw_ex(("%s: object MUST define onSpawn() method.", classname.c_str()));
}

Object * Object::spawnGrouped(const std::string &classname, const std::string &animation, const v3<float> &dpos, const GroupType type) {
	return World->spawnGrouped(this, classname, animation, dpos, type);
}

void Object::limitRotation(const float dt, const float speed, const bool rotate_even_stopped, const bool allow_backward) {
	const int dirs = getDirectionsNumber();
	if (dirs == 1)
		return;
	
	assert(dirs == 8 || dirs == 16);
	if (_velocity.is0()) {
		_direction.fromDirection(_direction_idx, dirs); 
		return;
	}

	if (dirs == 8) {
		_velocity.quantize8();
		_dst_direction = _velocity.getDirection8() - 1;
	} else {
		_velocity.quantize16();
		_dst_direction = _velocity.getDirection16() - 1;
	}
	assert(_dst_direction >= 0);
	if (_dst_direction == _direction_idx) {
		_rotation_time = 0;
		return;
	}
		
	const int half_dirs = dirs / 2;

	if (_rotation_time <= 0) {
		//was not rotated.
		if (allow_backward && (_dst_direction - _direction_idx + dirs) % dirs == half_dirs) {
			return;
		}
		
		_rotation_time = speed;
	}

	
	if (_rotation_time > 0) {
		_rotation_time -= dt;
		if (_rotation_time <= 0) {
			//rotate.
			int dd = _dst_direction - _direction_idx;
			if (dd < 0) 
				dd += dirs;
			dd = (dd > half_dirs) ? -1: 1;
			_direction_idx += dd;
			if (_direction_idx < 0) 
				_direction_idx += dirs;
			if (_direction_idx >= dirs)
				_direction_idx -= dirs;
			_rotation_time = (_direction_idx == _dst_direction)? 0: speed;
			//LOG_DEBUG(("dd = %d, _direction_idx = %d, _dst_direction = %d", dd, _direction_idx, _dst_direction));
		} 
		_velocity.fromDirection(_direction_idx, dirs);
		//LOG_DEBUG(("%d -> %g %g", _direction_idx, _velocity.x, _velocity.y));
	}

	if (rotate_even_stopped) {
		int d = math::abs<int>(_dst_direction - _direction_idx);
		if (d > 1 && d != dirs - 1) {
			_velocity.clear();
		} else _velocity.fromDirection(_direction_idx, dirs);
	} 
	_direction.fromDirection(_direction_idx, dirs); //fixme. remove it.
	//LOG_DEBUG(("direction = %g %g, velocity = %g %g", _direction.x, _direction.y, _velocity.x, _velocity.y));
}

//grouped object stuff

void Object::add(const std::string &name, Object *obj) {
	assert(obj != NULL);
	if (_group.find(name) != _group.end())
		throw_ex(("object '%s'(%s) was already added to group", name.c_str(), obj->classname.c_str()));
	_group.insert(Group::value_type(name, obj->getID()));
	obj->need_sync = true;
	need_sync = true;
}

Object *Object::get(const std::string &name) {
	Group::iterator i = _group.find(name);
	if (i == _group.end())
		throw_ex(("there's no object '%s' in group", name.c_str()));
	Object * o = World->getObjectByID(i->second);
	if (o == NULL)
		throw_ex(("%s: world doesnt know anything about '%s' [group]", classname.c_str(), name.c_str()));
	return o;
}

const Object *Object::get(const std::string &name) const {
	Group::const_iterator i = _group.find(name);
	if (i == _group.end())
		throw_ex(("there's no object '%s' in group", name.c_str()));
	Object * o = World->getObjectByID(i->second);
	if (o == NULL)
		throw_ex(("%s: world doesnt know anything about '%s' [group]", classname.c_str(), name.c_str()));
	return o;
}

const bool Object::has(const std::string &name) const {
	return _group.find(name) != _group.end();
}

void Object::remove(const std::string &name) {
	Group::iterator i = _group.find(name); 
	if (i == _group.end())
		return;
	
	Object * o = World->getObjectByID(i->second);
	if (o != NULL) {
		o->emit("death", this);
		o->need_sync = true;
	}
	_group.erase(i);
	need_sync = true;
}


void Object::groupEmit(const std::string &name, const std::string &event) {
	Group::const_iterator i = _group.find(name);
	if (i == _group.end())
		throw_ex(("there's no object '%s' in group", name.c_str()));
	Object * o = World->getObjectByID(i->second);
	if (o == NULL) {
		//throw_ex(("%s: world doesnt know anything about '%s', id: %d [group]", classname.c_str(), name.c_str(), i->second));
		LOG_ERROR(("%s: world doesnt know anything about '%s', id: %d [group] [attempt to recover]", classname.c_str(), name.c_str(), i->second));
		return;
	}
	o->emit(event, this);
}

//effects
void Object::addEffect(const std::string &name, const float ttl) {
	_effects[name] = ttl;
	need_sync = true;
}

const float Object::getEffectTimer(const std::string &name) const {
	EffectMap::const_iterator i = _effects.find(name);
	if (i == _effects.end())
		throw_ex(("getEffectTimer: object does not have effect '%s'", name.c_str()));
	return i->second;
}

void Object::removeEffect(const std::string &name) {
	_effects.erase(name);
	need_sync = true;
}

void Object::calculate(const float dt) {
	if (_follow > 0) {
		Object *leader = World->getObjectByID(_follow);
		if (leader) {
			_direction = leader->_direction;
			setDirection(leader->getDirection());
			return;
		}
	}
	_velocity.clear();
		
	if (_state.left) _velocity.x -= 1;
	if (_state.right) _velocity.x += 1;
	if (_state.up) _velocity.y -= 1;
	if (_state.down) _velocity.y += 1;
	
	_velocity.normalize();
}

const bool Object::getNearest(const std::vector<std::string> &targets, v3<float> &position, v3<float> &velocity) const {
	int found = 0;
	for(size_t i = 0; i < targets.size(); ++i) {
		v3<float> pos, vel;
		if (!getNearest(targets[i], pos, vel)) 
			continue;
		++found;
		if (found == 1 || pos.quick_length() < position.quick_length()) {
			position = pos; velocity = vel;
		}
	}
	
	return found != 0;	
}

const bool Object::findPath(v3<float> &position, Way &way) const {
	return World->findPath(this, position, way);
}


const std::string Object::getType() const {
	static const std::string empty;
	return empty;
}

const int Object::getCount() const {
	return 0;
}


void Object::getTargetPosition(v3<float> &relative_position, const v3<float> &target, const std::string &weapon) {
	const int dirs = _directions_n;
	
	const Object *wp = ResourceManager->getClass(weapon);
	float range = wp->ttl * wp->speed;
	
	GET_CONFIG_VALUE("engine.targeting-multiplier", float, tm, 0.8);
	if (tm <= 0 || tm >= 1) 
		throw_ex(("targeting multiplier must be greater than 0 and less than 1.0 (%g)", tm))
	range *= tm;
	double dist = target.length();
	if (dist > range) 
		dist = range;
	//LOG_DEBUG(("searching suitable position (distance: %g, range: %g)", dist, range));
	double distance = 0;
	
	for(int i = 0; i < dirs; ++i) {
		v3<float> pos;
		pos.fromDirection(i, dirs);
		pos *= dist;
		pos += target;
		double d = pos.quick_length();
		if (i == 0 || d < distance) {
			distance = d;
			relative_position = pos;
		}
		//LOG_DEBUG(("target position: %g %g, distance: %g", pos.x, pos.y, d));
	}
}

void Object::checkSurface() {
	if (_surface && _cmap) 
		return;
	ResourceManager->checkSurface(_surface_name, _surface, _cmap);
	assert(_surface != NULL);
	assert(_cmap != NULL);
}
