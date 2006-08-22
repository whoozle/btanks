#include "object.h"
#include "sdlx/surface.h"
#include "mrt/exception.h"
#include "mrt/logger.h"
#include "animation_model.h"
#include "resource_manager.h"
#include "world.h"

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


Object::Object(const std::string &classname, const bool stateless) : 
	BaseObject(classname, stateless),  _model(0), _surface(0), _direction_idx(0), _pos(0), _follow(0) {}

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


Object * Object::clone(const std::string &opt) const {
	throw_ex(("your object uses Object directly, which is obsoleted and prohibited."));
//	Object *obj = new Object(*this);
//	return obj;
	return NULL;
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
	
	if (!_way.empty()) {
		if (_distance > 0) {
			//moving
			//LOG_DEBUG(("%g %g %d %d, distance: %g", getPosition().x, getPosition().y, _way.begin()->x, _way.begin()->y, _distance));
			//LOG_DEBUG(("o_velocity: %g %g", _velocity.x, _velocity.y));
		} else {
			_velocity = _way.begin()->convert<float>();
			//LOG_DEBUG(("next waypoint: %g %g", _velocity.x, _velocity.y));
			_velocity -= getPosition();
			_distance = _velocity.length();
			_way.erase(_way.begin());
		}
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
	if (frame < 0 || frame >= (int)pose->frames.size()) {
		LOG_WARN(("%s: event '%s' frame %d is out of range (position: %g).", classname.c_str(), _events.front().name.c_str(), frame, _pos));
		return;		
	}
	frame = pose->frames[frame];
	
	if (frame * _th >= _surface->getHeight()) {
		LOG_WARN(("%s: event '%s' tile row %d is out of range.", classname.c_str(), _events.front().name.c_str(), frame));
		return;
	}
	
	sdlx::Rect src(_direction_idx * _tw, frame * _th, _tw, _th);
	surface.copyFrom(*_surface, src, x, y);
}

const std::string Object::getState() const {
	static std::string empty;
	if (_events.empty())
		return empty;
	return _events.front().name;
}

void Object::serialize(mrt::Serializator &s) const {
	BaseObject::serialize(s);
	
	int en = _events.size();
	s.add(en);
	
	EventQueue::const_iterator i = _events.begin();
	while(en--) {
		i->serialize(s);
	}
	
	s.add(_model_name);
	s.add(_surface_name);
	s.add(_tw);
	s.add(_th);
	s.add(_direction_idx);
	s.add(_pos);
}

void Object::deserialize(const mrt::Serializator &s) {
	BaseObject::deserialize(s);

	_events.clear();
	int en;
	s.get(en);
	while(en--) {
		Event e;
		e.deserialize(s);
		//LOG_DEBUG(("event: %s, repeat: %s", e.name.c_str(), e.repeat?"true":"false"));
		_events.push_back(e);
	}
	s.get(_model_name);
	s.get(_surface_name);
	s.get(_tw);
	s.get(_th);
	s.get(_direction_idx);
	s.get(_pos);

	_model = ResourceManager->getAnimationModel(_model_name);
	_surface = ResourceManager->getSurface(_surface_name);

	size.x = _tw; size.y = _th;
}

void Object::emit(const std::string &event, const BaseObject * emitter) {
	BaseObject::emit(event, emitter);
}

void Object::setWay(const Way & way) {
	_way = way;
	_distance = 0;
	if (!way.empty()) 
		LOG_DEBUG(("set %d pending waypoints", _way.size()));
}

const bool Object::isDriven() const {
	return !_way.empty();
}

void Object::follow(const Object *obj) {
	_follow = obj->_id;
}

void Object::follow(const int id) {
	_follow = id;
}

void Object::setup(const std::string &a) {
	ResourceManager->initMe(this, a);
	animation = a;
	
	memset(&_state, 0, sizeof(_state));
	_events.clear();
}
