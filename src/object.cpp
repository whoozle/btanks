#include "object.h"
#include "sdlx/surface.h"
#include "mrt/exception.h"
#include "mrt/logger.h"
#include "animation_model.h"
#include "resource_manager.h"
#include "world.h"
#include "math/abs.h"

Object::Event::Event() {}

Object::Event::Event(const std::string name, const bool repeat): 
	name(name), repeat(repeat) {}
	
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
	BaseObject(classname), fadeout_time(0),  _model(0), _surface(0), _direction_idx(0), _pos(0), _rotation_time(0) {}

void Object::init(const std::string &model, const std::string &surface, const int tile_w, const int tile_h) {
	_events.clear();

	_model = ResourceManager->getAnimationModel(model);
	_model_name = model;

	_surface = ResourceManager->getSurface(surface);
	_surface_name = surface;

	size.x = _tw = tile_w; size.y = _th = tile_h;
	_direction_idx = 0;
	_pos = 0;
}

void Object::init(const Object *a) {
	_model = a->_model;
	_model_name = a->_model_name;
	_surface = a->_surface;
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
	_direction_idx = dir;
}

const int Object::getDirection() const {
	return _direction_idx;
}

void Object::play(const std::string &id, const bool repeat) {
	if (_events.empty())
		_pos = 0;
	_events.push_back(Event(id, repeat));
}

void Object::playNow(const std::string &id) {
	_pos = 0;
	_events.push_front(Event(id, false));
}

void Object::cancel() {
	if (_events.empty()) 
		return;
	_events.pop_front();
	_pos = 0;
}

void Object::cancelRepeatable() {
	for (EventQueue::iterator i = _events.begin(); i != _events.end();) {
		if (i->repeat) {
			if (i == _events.begin())
				_pos = 0;
			i = _events.erase(i);
		} 
		else ++i;
	}
}


void Object::cancelAll() {
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
		++ei;
	}

	if (_events.empty()) 
		return;
	
	const Event & event = _events.front();
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

void Object::render(sdlx::Surface &surface, const int x, const int y) {
	if (_events.empty()) {
		if (!isDead())
			LOG_WARN(("%s: no animation played. latest position: %g", classname.c_str(), _pos));
		return;
	}
	const Pose * pose = _model->getPose(_events.front().name);
	if (pose == NULL) {
		LOG_WARN(("%s: pose '%s' is not supported", classname.c_str(), _events.front().name.c_str()));
		return; 
	}
	
	int frame = (int)_pos;
	int n = (int)pose->frames.size();
	if (n == 0) {
		LOG_WARN(("%s: pose '%s' doesnt have any frames", classname.c_str(), _events.front().name.c_str()));
		return; 
	}
	
	
	//this stuff need to be fixed, but I still cannot find cause for overflowing frame
	if (frame == n)
		frame = n - 1;
	
	if (frame < 0 || frame >= n) {
		LOG_WARN(("%s: event '%s' frame %d is out of range (position: %g).", classname.c_str(), _events.front().name.c_str(), frame, _pos));
		return;		
	}
	frame = pose->frames[frame];
	
	if (frame * _th >= _surface->getHeight()) {
		LOG_WARN(("%s: event '%s' tile row %d is out of range.", classname.c_str(), _events.front().name.c_str(), frame));
		return;
	}

	sdlx::Rect src(_direction_idx * _tw, frame * _th, _tw, _th);

	int alpha = 0;
	if (fadeout_time > 0 && ttl > 0 && ttl < fadeout_time) 
		alpha = (int)(255 * (fadeout_time - ttl) / fadeout_time);
	//LOG_DEBUG(("alpha = %d", alpha));
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


void Object::serialize(mrt::Serializator &s) const {
	BaseObject::serialize(s);
	
	s.add(animation);
	s.add(fadeout_time);
	
	int en = _events.size();
	s.add(en);
	{
		EventQueue::const_iterator i = _events.begin();
		while(en--) {
			i->serialize(s);
		}
	}

	en = _effects.size();
	s.add(en);
	{
		EffectMap::const_iterator i = _effects.begin();
		while(en--) {
			s.add(i->first);
			s.add(i->second);
		}
	}

	
	s.add(_model_name);
	s.add(_surface_name);
	s.add(_tw);
	s.add(_th);
	s.add(_direction_idx);
	s.add(_pos);

	//add support for waypoints here. AI cannot serialize now.
	s.add(_rotation_time);
	s.add(_dst_direction);

	//Group	
	en = _group.size();
	s.add(en);
	{
		Group::const_iterator i = _group.begin();
		while(en--) {
			s.add(i->first);
			s.add(i->second->_id);
		}
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
	s.get(_pos);

	s.get(_rotation_time);
	s.get(_dst_direction);

	_group.clear();
	s.get(en);
	while(en--) {
		std::string name;
		int id;
		s.get(name);
		s.get(id);
		Object *obj = World->getObjectByID(id);
		if (obj == NULL) 
			throw_ex(("object id %d was not created. bug in serialization order code.", id));
		_group[name] = obj;
	}
	//additional initialization
	_model = ResourceManager->getAnimationModel(_model_name);
	_surface = ResourceManager->getSurface(_surface_name);
}

void Object::emit(const std::string &event, BaseObject * emitter) {
	if (event == "death") {
		for(Group::iterator i = _group.begin(); i != _group.end(); ++i) {
			i->second->emit(event, this);
		}
		_group.clear();
	}
	BaseObject::emit(event, emitter);
}

void Object::setWay(const Way & way) {
	_way = way;
	_next_target.clear();
	if (!way.empty()) 
		LOG_DEBUG(("set %d pending waypoints", _way.size()));
}

void Object::calculateWayVelocity() {
	while (!_way.empty()) {
		if (_next_target.is0()) {
			_next_target = _way.begin()->convert<float>();
			_next_target_rel = _next_target - getPosition();
			LOG_DEBUG(("next waypoint: %g %g, relative: %g %g", _next_target.x, _next_target.y, _next_target_rel.x, _next_target_rel.y));
			_way.erase(_way.begin());
			break;
		}
		
		_velocity = _next_target - getPosition();
		if (_velocity.is0() || _velocity.x * _next_target_rel.x < 0 ||  _velocity.y * _next_target_rel.y < 0) {
			//wiping out way point and restart
			_next_target.clear();
		} else break;
	}
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

void Object::renderCopy(sdlx::Surface &surface) {
	const_cast<sdlx::Surface *>(_surface)->setAlpha(0,0);
	render(surface, 0, 0);
	const_cast<sdlx::Surface *>(_surface)->setAlpha(0, SDL_SRCALPHA);
}

void Object::limitRotation(const float dt, const int dirs, const float speed, const bool rotate_even_stopped, const bool allow_backward) {
	assert(dirs == 8 || dirs == 16);
	if (_velocity.is0()) {
		_direction.fromDirection(_direction_idx, dirs); 
		return;
	}
	_velocity.normalize();
	
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

	if (rotate_even_stopped) {
		int d = math::abs<int>(_dst_direction - _direction_idx);
		if (d > 1 && d != dirs - 1) {
			_velocity.clear();
		} else {
			_direction_idx = _dst_direction;
			_rotation_time = 0;
		}
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
		if (!rotate_even_stopped) {
			_velocity.fromDirection(_direction_idx, dirs);
			//LOG_DEBUG(("%d -> %g %g", _direction_idx, _velocity.x, _velocity.y));
		}
	}
	_direction.fromDirection(_direction_idx, dirs); //fixme. remove it.
	//LOG_DEBUG(("direction = %g %g, velocity = %g %g", _direction.x, _direction.y, _velocity.x, _velocity.y));
}

//grouped object stuff

void Object::add(const std::string &name, Object *obj) {
	if (_group.find(name) != _group.end())
		throw_ex(("object '%s'(%s) was already added to group", name.c_str(), obj->classname.c_str()));
	_group.insert(Group::value_type(name, obj));
}

Object *Object::get(const std::string &name) {
	Group::iterator i = _group.find(name);
	if (i == _group.end())
		throw_ex(("there's no object '%s' in group", name.c_str()));
	return i->second;
}

const Object *Object::get(const std::string &name) const {
	Group::const_iterator i = _group.find(name);
	if (i == _group.end())
		throw_ex(("there's no object '%s' in group", name.c_str()));
	return i->second;
}

void Object::groupEmit(const std::string &name, const std::string &event) {
	Object *o = get(name);
	o->emit(event, this);
}

//effects
void Object::addEffect(const std::string &name, const float ttl) {
	_effects[name] = ttl;
	need_sync = true;
}

void Object::removeEffect(const std::string &name) {
	_effects.erase(name);
	need_sync = true;
}
