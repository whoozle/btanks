
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

#include "object.h"
#include "config.h"
#include "sdlx/surface.h"
#include "sdlx/c_map.h"
#include "mrt/exception.h"
#include "mrt/logger.h"
#include "animation_model.h"
#include "resource_manager.h"
#include "world.h"
#include "tmx/map.h"
#include "math/unary.h"
#include "sound/mixer.h"
#include "special_owners.h"

Object::Event::Event() : name(), repeat(false), sound(), gain(1.0f), played(false), cached_pose(NULL) {}

Object::Event::Event(const std::string name, const bool repeat, const std::string &sound, const float gain, const Pose * p): 
	name(name), repeat(repeat), sound(sound), gain(gain), played(false), cached_pose(p) {}
	
void Object::Event::serialize(mrt::Serializator &s) const {
	s.add(name);
	s.add(repeat);
}
void Object::Event::deserialize(const mrt::Serializator &s) {
	s.get(name);
	s.get(repeat);
}

Object * Object::clone() const {
	throw_ex(("object %s:%s doesnt provide clone() method", registered_name.c_str(), animation.c_str()));
	return NULL;
}

Object::Object(const std::string &classname) : 
	BaseObject(classname), 
	registered_name(), animation(), fadeout_time(0),  
	_animation(0), _model(0), 
	_surface(0), _fadeout_surface(0), _fadeout_alpha(0), _cmap(0), 
	_events(), _effects(), 
	_tw(0), _th(0), _direction_idx(0), _directions_n(8), _pos(0), 
	_way(), _next_target(), _next_target_rel(), 
	_rotation_time(0), 
	_dst_direction(-1), 
	_group(), _blinking(true)
	 {
	 	GET_CONFIG_VALUE("engine.spawn-invulnerability-blinking-interval", float, ibi, 0.3);
	 	_blinking.set(ibi);
	 }

Object::~Object() { Mixer->cancelAll(this); delete _fadeout_surface; }

void Object::init(const Animation *a) {
	_animation = a;
	_model = ResourceManager->getAnimationModel(a->model);
	
	_surface = ResourceManager->getSurface(a->surface);
	_cmap = ResourceManager->getCollisionMap(a->surface);

	_tw = a->tw;
	_th = a->th;
	
	size.x = _tw;
	size.y = _th;
}


Object* Object::spawn(const std::string &classname, const std::string &animation, const v2<float> &dpos, const v2<float> &vel, const int z) {
	return World->spawn(this, classname, animation, dpos, vel, z);
}


#include "game_monitor.h"

const Object* Object::getNearestObject(const std::string &classname) const {
	if (GameMonitor->disabled(this))
		return NULL;
	return World->getNearestObject(this, classname);
}

const Object* Object::getNearestObject(const std::set<std::string> &classnames) const {
	if (GameMonitor->disabled(this))
		return NULL;
	return World->getNearestObject(this, classnames);
}

const bool Object::getNearest(const std::string &cl, v2<float> &position, v2<float> &velocity, Way * way) const {
	if (GameMonitor->disabled(this))
		return false;
	return World->getNearest(this, cl, position, velocity, way);
}

const bool Object::getNearest(const std::set<std::string> &classnames, v2<float> &position, v2<float> &velocity) const {
	if (GameMonitor->disabled(this))
		return false;
	return World->getNearest(this, classnames, position, velocity);
}

const bool Object::getNearest(const std::set<std::string> &classnames, const float range, v2<float> &position, v2<float> &velocity) const {
	if (GameMonitor->disabled(this))
		return false;
	
	return World->getNearest(this, classnames, range, position, velocity);
}


void Object::setDirection(const int dir) {
	if (dir >= _directions_n)
		LOG_WARN(("%s:%s setDirection(%d) called on object with %d directions", registered_name.c_str(), animation.c_str(), dir, _directions_n));
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
	} else throw_ex(("%s:%s cannot handle %d directions", registered_name.c_str(), animation.c_str(), _directions_n));
	setDirection(dir - 1);
}


void Object::play(const std::string &id, const bool repeat) {
	if (_events.empty())
		_pos = 0;
	checkAnimation();
	const Pose *pose = _model->getPose(id);
	if (pose == NULL) {
		LOG_WARN(("animation model %s does not have pose '%s'", _animation->model.c_str(), id.c_str()));
		return;
	}

	_events.push_back(Event(id, repeat, pose->sound, pose->gain, pose));
}

void Object::playNow(const std::string &id) {
	checkAnimation();

	const Pose *pose = _model->getPose(id);
	if (pose == NULL) {
		LOG_WARN(("animation model %s does not have pose %s", _animation->model.c_str(), id.c_str()));
		return;
	}
	_pos = 0;
	_events.push_front(Event(id, false, pose->sound, pose->gain, pose));
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
			if (!_velocity.is0()) {
				_direction = _velocity;
				_velocity.clear();
			}
		} else if (ei->first == "invulnerability") {
			_blinking.tick(dt);
		}
		++ei;
	}

	if (_events.empty()) 
		return;
	
	Event & event = _events.front();
	//LOG_DEBUG(("%p: event: %s, pos = %f", (void *)this, event.name.c_str(), _pos));
	const Pose * pose = event.cached_pose;
	if (pose == NULL) {
		checkAnimation();
		event.cached_pose = pose = _model->getPose(event.name);
	}
	
	if (pose == NULL) {
		LOG_WARN(("animation model %s does not have pose %s", _animation->model.c_str(), event.name.c_str()));
		cancel();
		return;
	}
	
	if (pose->z > -1000) {
		setZ(pose->z);
	}
	
	if (!event.played) {
		event.played = true;
		if (!event.sound.empty()) {
			if (event.sound[0] != '@') {
				Mixer->playSample(this, event.sound, event.repeat, event.gain);
			} else {
				Mixer->playRandomSample(this, event.sound.substr(1), event.repeat, event.gain);
			}
		}
	}
	
	_pos += dt * pose->speed;
	int n = pose->frames.size();
	if (n == 0) {
		LOG_WARN(("animation model %s, pose %s doesnt contain any frames", _animation->model.c_str(), event.name.c_str()));
		return;
	}
		
	int cycles = (int)_pos / n;
	//LOG_DEBUG(("%s: _pos: %f, cycles: %d", classname.c_str(), _pos, cycles));
	_pos -= cycles * n;
	if (_pos < 0)
		_pos += n;
	if (_pos >= n) 
		_pos -= n;
	
	if (cycles) {
		if (!event.repeat)
			cancel();
	} 
}

void Object::playSound(const std::string &name, const bool loop, const float gain) {
	Mixer->playSample(this, name + ".ogg", loop, gain);
}

void Object::playRandomSound(const std::string &classname, const bool loop, const float gain) {
	Mixer->playRandomSample(this, classname, loop, gain);
}


const bool Object::getRenderRect(sdlx::Rect &src) const {
	if (_events.empty()) {
		if (!isDead())
			LOG_WARN(("%s: no animation played. latest position: %g", registered_name.c_str(), _pos));
		return false;
	}

	const Event & event = _events.front();
	const Pose * pose = event.cached_pose;
	if (pose == NULL) {
		checkAnimation();
		event.cached_pose = pose = _model->getPose(event.name);
	}

	if (pose == NULL) {
		LOG_WARN(("%s:%s pose '%s' is not supported", registered_name.c_str(), animation.c_str(), _events.front().name.c_str()));
		return false;
	}
	
	int frame = (int)_pos;
	int n = (int)pose->frames.size();
	if (n == 0) {
		LOG_WARN(("%s:%s pose '%s' doesnt have any frames", registered_name.c_str(), animation.c_str(), _events.front().name.c_str()));
		return false;
	}
	
	//this stuff need to be fixed, but I still cannot find cause for overflowing frame
	if (frame >= n)
		frame = n - 1;
	
	if (frame < 0 || frame >= n) {
		LOG_WARN(("%s:%s  event '%s' frame %d is out of range (position: %g).", registered_name.c_str(), animation.c_str(), _events.front().name.c_str(), frame, _pos));
		return false;	
	}

	frame = pose->frames[frame];
	
	
	checkSurface();
	
	if (frame * _th >= _surface->getHeight()) {
		LOG_WARN(("%s:%s event '%s' tile row %d is out of range.", registered_name.c_str(), animation.c_str(), _events.front().name.c_str(), frame));
		return false;
	}

	src.x = _direction_idx * _tw;
	src.y = frame * _th;
	src.w = _tw;
	src.h = _th;
	return true;
}

const bool Object::skipRendering() const {
	if (_follow != 0) {
		Object *leader = World->getObjectByID(_follow);
		if (leader != NULL)
			return leader->skipRendering();
	}
	if (!isEffectActive("invulnerability"))
		return false;
	if (getEffectTimer("invulnerability") == -1) 
		return false;
	return _blinking.get() >= 0.5;
}


void Object::render(sdlx::Surface &surface, const int x, const int y) {
	if (skipRendering()) {
		return;
	}

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
	alpha = 255 - alpha;
	
	GET_CONFIG_VALUE("engine.fadeout-strip-alpha-bits", int, strip_alpha_bits, 4);
	alpha &= ~((1 << strip_alpha_bits) - 1);
	
	if (_fadeout_surface != NULL && alpha == _fadeout_alpha) {
		surface.copyFrom(*_fadeout_surface, x, y);
		//LOG_DEBUG(("skipped all fadeout stuff"));
		return;
	}
	_fadeout_alpha = alpha;
	
	if (_fadeout_surface == NULL) {
		_fadeout_surface = new sdlx::Surface;
		_fadeout_surface->createRGB(_tw, _th, 32, SDL_SWSURFACE);
		_fadeout_surface->convertAlpha();
	}
	
	const_cast<sdlx::Surface *>(_surface)->setAlpha(0,0);
	_fadeout_surface->copyFrom(*_surface, src);
	const_cast<sdlx::Surface *>(_surface)->setAlpha(0);

	SDL_Surface *s = _fadeout_surface->getSDLSurface();
	assert(s->format->BytesPerPixel > 2);

	_fadeout_surface->lock();
	//optimize it.
	Uint32 *p = (Uint32 *)s->pixels;
	int size = s->h * s->pitch / 4;
	for(int i = 0; i < size; ++i) {
		Uint8 r, g, b, a;
		_fadeout_surface->getRGBA(*p, r, g, b, a);
		if (a == 0) {
			++p;
			continue;
		}
		a = (((int)a) * alpha) / 255;
		*p++ = _fadeout_surface->mapRGBA(r, g, b, a);
	}
	_fadeout_surface->unlock();

	surface.copyFrom(*_fadeout_surface, x, y);
}

const bool Object::collides(const Object *other, const int x, const int y, const bool hidden_by_other) const {
	sdlx::Rect src, other_src;
	assert(other != NULL);
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
	checkSurface();
	other->checkSurface();
	
	return _cmap->collides(src, other->_cmap, other_src, x, y, hidden_by_other);
}

const bool Object::collides(const sdlx::CollisionMap *other, const int x, const int y, const bool hidden_by_other) const {
	assert(other != NULL);
	sdlx::Rect src;
	if (!getRenderRect(src)) 
		return false;

	checkSurface();
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
	
	_blinking.serialize(s);
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
	
	s.get(_tw);
	s.get(_th);
	s.get(_direction_idx);
	s.get(_directions_n);
	s.get(_pos);

	s.get(en);
	_way.clear();
	while(en--) {
		v2<int> wp;
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
	_blinking.deserialize(s);

	checkAnimation();
}

void Object::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
		for(Group::iterator i = _group.begin(); i != _group.end(); ++i) {
			Object * o = World->getObjectByID(i->second);
			if (o == NULL)
				continue;
			o->emit(event, emitter);
		}
		_group.clear();
		_velocity.clear();
		_dead = true;
	} else if (event == "collision") {
		if (piercing && emitter != NULL)
			emitter->addDamage(this);
		//addDamage(emitter);
	} else 
		LOG_WARN(("%s[%d]: unhandled event '%s'", registered_name.c_str(), _id, event.c_str()));
}

void Object::setWay(const Way & way) {
	_way = way;
	_next_target.clear();
	if (!way.empty()) { 
		LOG_DEBUG(("%d:%s:%s set %u pending waypoints", getID(), registered_name.c_str(), animation.c_str(), (unsigned)_way.size()));
		_next_target = way.begin()->convert<float>();
	}
}

void Object::calculateWayVelocity() {
	v2<float> position = getPosition();
	sdlx::Rect me((int)position.x, (int)position.y, (int)size.x, (int)size.y);

	while (!_way.empty()) {
		_velocity.clear();
		
		if (_next_target.is0()) {
			_next_target = _way.begin()->convert<float>();
			v2<float> rel = _next_target - position;
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
		GET_CONFIG_VALUE("engine.allowed-pathfinding-fault", int, f, 5);
		if ((_next_target_rel.x != 0 && _velocity.x * _next_target_rel.x <= 0) || (math::abs(_velocity.x) < f))
			_velocity.x = 0;
		if ((_next_target_rel.y != 0 && _velocity.y * _next_target_rel.y <= 0) || (math::abs(_velocity.y) < f))
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

void Object::init(const std::string &a) {
	init(ResourceManager.get_const()->getAnimation(a));
	animation = a;
}

void Object::onSpawn() {
	throw_ex(("%s: object MUST define onSpawn() method.", registered_name.c_str()));
}

Object * Object::spawnGrouped(const std::string &classname, const std::string &animation, const v2<float> &dpos, const GroupType type) {
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
		int d = _velocity.getDirection8() - 1;
		if (d >= 0) 
			_dst_direction = d;
	} else {
		_velocity.quantize16();
		int d = _velocity.getDirection16() - 1;
		if (d >= 0) 
			_dst_direction = d;
	}
	if (_dst_direction < 0)
		return;
	
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
		throw_ex(("%s: world doesnt know anything about '%s' [group]", registered_name.c_str(), name.c_str()));
	return o;
}

const Object *Object::get(const std::string &name) const {
	Group::const_iterator i = _group.find(name);
	if (i == _group.end())
		throw_ex(("there's no object '%s' in group", name.c_str()));
	Object * o = World->getObjectByID(i->second);
	if (o == NULL)
		throw_ex(("%s: world doesnt know anything about '%s' [group]", registered_name.c_str(), name.c_str()));
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
		LOG_ERROR(("%s: world doesnt know anything about '%s', id: %d [group] [attempt to recover]", registered_name.c_str(), name.c_str(), i->second));
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


const bool Object::old_findPath(const v2<float> &position, Way &way) const {
	return World->old_findPath(this, position, way);
}

const bool Object::old_findPath(const Object *target, Way &way) const {
	return World->old_findPath(this, getRelativePosition(target), way, target);
}

const std::string Object::getType() const {
	static const std::string empty;
	return empty;
}

const int Object::getCount() const {
	return 0;
}


const float Object::getWeaponRange(const std::string &weapon) const {
	const Object *wp = ResourceManager->getClass(weapon);
	float range = wp->ttl * wp->speed;
	
	float tm;
	Config->get("objects." + registered_name + ".targeting-multiplier", tm, 0.5);
	
	if (tm <= 0 || tm >= 1) 
		throw_ex(("targeting multiplier must be greater than 0 and less than 1.0 (%g)", tm));
	return range * tm;
}

#include "math/vector.h"

const int Object::getTargetPosition(v2<float> &relative_position, const std::set<std::string> &targets, const std::string &weapon) const {
	float range = getWeaponRange(weapon);
	return getTargetPosition(relative_position, targets, range);
}

const int Object::getTargetPosition(v2<float> &relative_position, const std::set<std::string> &targets, const float range) const {
	if (GameMonitor->disabled(this))
		return -1;

	const int dirs = _directions_n;
	
	std::set<const Object *> objects;
	World->enumerateObjects(objects, this, range, &targets);
	
	v2<int> pfs = Map->getPathTileSize();
	const Matrix<int> &matrix = getImpassabilityMatrix();
//		v2<int> map_pos = (pos + getPosition()).convert<int>() / pfs;

	int result_dir = -1;
	float distance = -1; //no result if it was bug. ;)

	for(int d = 0; d < dirs; ++d) {
		v2<float> dir;
		dir.fromDirection(d, dirs);
		for(std::set<const Object *>::const_iterator i = objects.begin(); i != objects.end(); ++i) {
			const Object *o = *i;
			if (hasSameOwner(o))
				continue;
			
			v2<float> pos, tp = getRelativePosition(o);
			if (!tp.same_sign(dir))
				continue;
			
			math::getNormalVector(pos, dir, tp);
			if (pos.quick_length() > tp.quick_length())
				continue;
			
			
			//skip solid objects
			if (impassability >= 1.0f) {
				// i am solid object. 
				v2<int> map_pos = (pos + getPosition()).convert<int>() / pfs;
				if (matrix.get(map_pos.y, map_pos.x) < 0)
					continue;
			}
			
			float dist = pos.quick_length();
			if (result_dir != -1 && dist >= distance)
				continue;


			if (impassability >= 1.0f) {
				//checking map projection
				v2<float> map1 = pos + getPosition();
				v2<float> map2 = o->getPosition();
			
				v2<float> dp (map2.x - map1.x, map2.y - map1.y);
				dp.normalize(pfs.x);
				if (dp.is0())
					continue;
			
				//LOG_DEBUG(("%g:%g -> %g:%g (+%g:+%g)", map1.x, map1.y, map2.x, map2.y, dp.x, dp.y));
				do {
					v2<float> dv = (map2 - map1) * dp; 
					if (dv.x < 0 || dv.y < 0) 
						break;
					map1 += dp;
					v2<int> map_pos = map1.convert<int>() / pfs;
					//LOG_DEBUG(("%dx%d: %d", map_pos.x, map_pos.y, matrix.get(map_pos.y, map_pos.x)));
					if (matrix.get(map_pos.y, map_pos.x) < 0)
						goto failed;
				} while(true);
				//end of map proj
			} //impassability >= 1.0f
			
			if (result_dir == -1 || dist < distance) {
				result_dir = d;
				distance = dist;
				relative_position = pos;
				//LOG_DEBUG(("enemy @ %g %g: %s (dir: %d, distance: %g)", pos.x, pos.y, o->registered_name.c_str(), d, distance));
			}
			
			failed: ;
		}
	}
	return result_dir;
}


const bool Object::getTargetPosition(v2<float> &relative_position, const v2<float> &target, const std::string &weapon) const {
	if (GameMonitor->disabled(this))
		return -1;

	const int dirs = _directions_n;
	
	float range = getWeaponRange(weapon);
	
	double dist = target.length();
	if (dist > range) 
		dist = range;
	
	//LOG_DEBUG(("searching suitable position (distance: %g, range: %g)", dist, range));
	double distance = 0;
	bool found = false;
	
	v2<int> pfs = Map->getPathTileSize();
	const Matrix<int> &matrix = getImpassabilityMatrix();
	
	for(int i = 0; i < dirs; ++i) {
		v2<float> pos;
		pos.fromDirection(i, dirs);
		pos *= dist;
		pos += target;
		double d = pos.quick_length();
		v2<int> map_pos = (pos + getPosition()).convert<int>() / pfs;
		if (matrix.get(map_pos.y, map_pos.x) < 0)
			continue;
		
		if (!found || d < distance) {
			distance = d;
			relative_position = pos;
			found = true;
		}
		//LOG_DEBUG(("target position: %g %g, distance: %g", pos.x, pos.y, d));
	}
	return found;
}

void Object::checkAnimation() const {
	if (_animation && _model)
		return;
	_animation = ResourceManager.get_const()->getAnimation(animation);
	_model = ResourceManager->getAnimationModel(_animation->model);
}


void Object::checkSurface() const {
	if (_surface && _cmap) 
		return;
	Object *nc_this = const_cast<Object *>(this);
	ResourceManager->checkSurface(animation, nc_this->_surface, nc_this->_cmap);
	assert(_surface != NULL);
	assert(_cmap != NULL);
}

void Object::close(const v2<int> & vertex) {
		_close_list.insert(vertex);
/*
		_close_list.insert(vertex-1);
		_close_list.insert(vertex+1);

		_close_list.insert(_pitch + vertex-1);
		_close_list.insert(_pitch + vertex);
		_close_list.insert(_pitch + vertex+1);

		_close_list.insert(-_pitch + vertex-1);
		_close_list.insert(-_pitch + vertex);
		_close_list.insert(-_pitch + vertex+1);
*/
}

static inline const int h(const v2<int>& src, const v2<int>& dst) {
	return 500 * (math::abs(src.x - dst.x) + math::abs<int>(src.y - dst.y));
}


void Object::findPath(const v2<int> target, const int step) {
	_step = step;
	_end = target;
	getPosition(_begin);

	_begin /= step;
	_end /= step;
	
	LOG_DEBUG(("%s[%d]: findPath %d:%d -> %d:%d", registered_name.c_str(), getID(), _begin.x, _begin.y, _end.x, _end.y));
	
	//while(!_open_list.empty())
	//	_open_list.pop();
	_open_list = OpenList();
	
	_close_list.clear();
	_points.clear();
	
	
	Point p;
	p.id = _begin;
	p.g = 0;
	p.h = h(p.id, _end);
	p.dir = getDirection();

	_open_list.push(PD(p.g + p.h, p.id));
	_points[p.id] = p;
}

#include "player_manager.h"

const bool Object::findPathDone(Way &way) {
	if (PlayerManager->isClient()) 
		return false;
	
	if (_begin == _end) {
		way.clear();
		way.push_back(_end);
		LOG_DEBUG(("append %d %d to way. 1 point-way.", _end.x, _end.y));
		_open_list = OpenList();
		return true;
	}
	const v2<int> map_size = Map->getSize();
	int dir_save = getDirection();
	GET_CONFIG_VALUE("engine.pathfinding-slice", int, ps, 1);
	GET_CONFIG_VALUE("engine.pathfinding-throttling", bool, pt, true);
	
	while(!_open_list.empty() && (pt?ps--:ps) > 0) {
		PointMap::const_iterator pi = _points.find(_open_list.top().id);
		assert(pi != _points.end());
		const Point& current = pi->second;
		assert(pi->first == current.id);
		_open_list.pop();
		
		if (_close_list.find(current.id) != _close_list.end())
			continue;
/*
		LOG_DEBUG(("%d: popping vertex. id=%d, x=%d, y=%d, g=%d, h=%d, f=%d", getID(), 
			current.id, current.id % _pitch, current.id / _pitch, current.g, current.h, current.g + current.h));
*/		
		_close_list.insert(current.id);
		const int x = current.id.x * _step;
		const int y = current.id.y * _step;
		
		//if (registered_name.substr(0, 2) == "ai")
			//LOG_DEBUG(("%s: testing node at %d,%d, value = g: %d, h: %d, f: %d", registered_name.c_str(), current.id.x, current.id.y, current.g, current.h, current.g + current.h));

		//searching surrounds 
		assert(current.dir != -1);
		const int dirs = getDirectionsNumber();
		if (dirs < 4 || dirs > 8)
			throw_ex(("pathfinding cannot handle directions number: %d", dirs));
			
		for(int i = 0; i < dirs; ++i) {
			v2<float> d;
			d.fromDirection(i, dirs);
			d.x = math::sign(d.x) * _step;
			d.y = math::sign(d.y) * _step;
			
			d.x += x;
			d.y += y;
			
			if (d.x < 0 || d.x >= map_size.x || d.y < 0 || d.y >= map_size.y)
				continue;
			
			v2<int> id((int)(d.x / _step), (int)(d.y / _step));
			
			assert( id != current.id );
			
			if (_close_list.find(id) != _close_list.end())
				continue;
	
	
			setDirection(i);
			v2<int> world_pos(id.x * _step, id.y * _step);
			int map_im = Map->getImpassability(this, world_pos);
			//LOG_DEBUG(("%d, %d, map: %d", world_pos.x, world_pos.y, map_im));
			assert(map_im >= 0);
			if (map_im >= 100) {
				//_close_list.insert(id);
				close(id);
				continue;			
			}
			float im = World->getImpassability(this, world_pos, NULL, true, true);
			//LOG_DEBUG(("%d, %d, world: %g", pos.x, pos.y, im));
			assert(im >= 0);
			if (im >= 1.0) {
				//_close_list.insert(id);
				close(id);
				continue;
			}
			
			Point p;
			p.id = id;
			p.dir = i;
			p.parent = current.id;
			p.g = current.g + ((d.x != 0 && d.y != 0)?141:100) + (int)(im * 100) + map_im;
			p.h = h(id, _end);


			//add penalty for turning

			/*
			int dd = math::abs(i - current.dir);
			if (dd > dirs/2) 
				dd = dirs - dd;
			p.h += 50 * dd;
			//car-specific penalties.
			if (map_im > 10 || im > 0.1) 
				p.g += map_im * 30 + (int)(im * 100) * 30;
			
			*/
			
			//LOG_DEBUG(("%s: appending %d at %d %d value = g: %d, h: %d, f: %d", registered_name.c_str(), p.id, pos.x, pos.y, p.g, p.h, p.g + p.h));
			
			PointMap::iterator pi = _points.find(id);
			
			if (pi != _points.end()) {
				if (pi->second.g > p.g) {
					pi->second = p;
				}
			} else 
				_points.insert(PointMap::value_type(id, p));
			
			
			if (p.h < 100) {
				_end = p.id;
				goto found;
			}

			_open_list.push(PD(p.g + p.h, p.id));
		}
		if (!pt) { 
			--ps;
			if (ps == 0)
				ps = -1; 
		} 
	}
	
	setDirection(dir_save);

	if (ps < 0) {
		return false;
	}

	way.clear();
	return true;

found:
	way.clear();
	
	_open_list = OpenList();
	//while(!_open_list.empty())
	//	_open_list.pop();
	
	_close_list.clear();
	
	setDirection(dir_save);

	for(v2<int> id = _end; id != _begin; ) {
		Point &p = _points[id];
		way.push_front(p.id * _step);
		//LOG_DEBUG(("%dx%d -> %dx%d", p.id % _pitch, p.id / _pitch, way.front().x, way.front().y));
		assert(id != p.parent);
		id = p.parent;
	}
	_points.clear();

	return true;
}

#include "game.h"

const std::string Object::getNearestWaypoint(const std::string &name) const {
	return Game->getNearestWaypoint(this, name);
}

void Object::addDamage(Object *from, const bool emitDeath) {
	if (from == NULL || !from->piercing)
		return;
	if (from->hasOwner(OWNER_COOPERATIVE) && hasOwner(OWNER_COOPERATIVE))
		return;
	addDamage(from, from->max_hp, emitDeath);
}

void Object::addDamage(Object *from, const int d, const bool emitDeath) {
	if (hp == -1 || d == 0 || from == NULL)
		return;
	if (isEffectActive("invulnerability"))
		return;
	
	int damage = d;
	/*
	GET_CONFIG_VALUE("engine.damage-randomization", float, dr, 0.3);
	int radius = (int)(damage * dr);
	if (radius > 0) {
		damage += mrt::random(radius * 2 + 1) - radius;
	}
	*/
	need_sync = true;
	
	hp -= damage;	
	//LOG_DEBUG(("%s: received %d hp of damage from %s. hp = %d", registered_name.c_str(), damage, from->classname.c_str(), hp));
	if (emitDeath && hp <= 0) 
		emit("death", from);
		
	//look for a better place for that.
	if (piercing)
		return;
	
	Object *o = ResourceManager->createObject("damage-digits", "damage-digits");
	o->hp = damage;
	if (hp < 0) 
		o->hp += hp;
	v2<float> pos;
	getPosition(pos);
	World->addObject(o, pos);
	o->setZ(getZ() + 1, true);
}

const sdlx::Surface * Object::getSurface() const {
	checkSurface();
	return _surface;
}

const float Object::getStateProgress() const {
	if (_events.empty()) 
		return 0;

	const Event & event = _events.front();
	//LOG_DEBUG(("%p: event: %s, pos = %f", (void *)this, event.name.c_str(), _pos));
	const Pose * pose = event.cached_pose;
	if (pose == NULL) { 
		checkAnimation();
		event.cached_pose = pose = _model->getPose(event.name);
	}
	
	if (pose == NULL) {
		return 0;
	}
	
	const float progress = _pos / pose->frames.size();

	return progress > 1.0 ? 1.0 : progress;
}

#include "zbox.h"

void Object::setZBox(const int zb) {
	need_sync = true;
	
	LOG_DEBUG(("%s::setZBox(%d)", registered_name.c_str(), zb));
	int z = getZ();
	z -= ZBox::getBoxBase(z); //removing current box
	z += ZBox::getBoxBase(zb);
	setZ(z, true);
	
	for(Group::const_iterator i = _group.begin(); i != _group.end(); ++i) {
		Object *o = World->getObjectByID(i->second);
		if (o != NULL)
			o->setZBox(zb);
	}
}

const Matrix<int> &Object::getImpassabilityMatrix() const {
	return Map->getImpassabilityMatrix(getZ());
}
