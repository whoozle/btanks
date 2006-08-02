#include "animated_object.h"
#include "sdlx/surface.h"
#include "mrt/exception.h"
#include "mrt/logger.h"
#include "animation_model.h"
#include "resource_manager.h"

AnimatedObject::Event::Event() {}

AnimatedObject::Event::Event(const std::string name, const bool repeat): 
	name(name), repeat(repeat) {}
	
void AnimatedObject::Event::serialize(mrt::Serializator &s) const {
	s.add(name);
	s.add(repeat);
}
void AnimatedObject::Event::deserialize(const mrt::Serializator &s) {
	s.get(name);
	s.get(repeat);
}


AnimatedObject::AnimatedObject(const std::string &classname) : 
	Object(classname),  _model(0), _surface(0), _direction_idx(0), _pos(0)  {}

void AnimatedObject::init(const std::string &model, const std::string &surface, const int tile_w, const int tile_h) {
	_events.clear();

	_model = ResourceManager->getAnimationModel(model);
	_model_name = model;

	_surface = ResourceManager->getSurface(surface);
	_surface_name = surface;

	size.x = _tw = tile_w; size.y = _th = tile_h;
	_direction_idx = 0;
	_pos = 0;
}

void AnimatedObject::init(const AnimatedObject &o) {
	//LOG_DEBUG(("classname: %s, model_name: %s", classname.c_str(), _model_name.c_str()));
	//LOG_DEBUG(("o.classname: %s, o.model_name: %s", o.classname.c_str(), o._model_name.c_str()));
	*this = o;
	//LOG_DEBUG(("classname: %s, model_name: %s", classname.c_str(), _model_name.c_str()));
/*
	_model_name = o._model_name;
	_model = o._model;
	_surface = o._surface;
	size.x = _tw = o._tw; size.y = _th = o._th;
	_direction_idx = o._direction_idx;
	_pos = o._pos;
*/
}



Object * AnimatedObject::clone(const std::string &opt) const {
	throw_ex(("your object uses AnimatedObject directly, which is obsoleted and prohibited."));
//	AnimatedObject *obj = new AnimatedObject(*this);
//	return obj;
	return NULL;
}


void AnimatedObject::setDirection(const int dir) {
	_direction_idx = dir;
}

const int AnimatedObject::getDirection() const {
	return _direction_idx;
}

void AnimatedObject::play(const std::string &id, const bool repeat) {
	if (_events.empty())
		_pos = 0;
	_events.push_back(Event(id, repeat));
}

void AnimatedObject::playNow(const std::string &id) {
	_pos = 0;
	_events.push_front(Event(id, false));
}

void AnimatedObject::cancel() {
	if (_events.empty()) 
		return;
	_events.pop_front();
	_pos = 0;
}

void AnimatedObject::cancelRepeatable() {
	for (EventQueue::iterator i = _events.begin(); i != _events.end();) {
		if (i->repeat) {
			if (i == _events.begin())
				_pos = 0;
			i = _events.erase(i);
		} 
		else ++i;
	}
}


void AnimatedObject::cancelAll() {
	while(!_events.empty())
		_events.pop_front();
}



void AnimatedObject::tick(const float dt) {
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
	int cycles = ((int)_pos / pose->frames.size());
	//LOG_DEBUG(("%s: _pos: %f, cycles: %d", classname.c_str(), _pos, cycles));
	
	if (cycles) {
		if (!event.repeat)
			cancel();
		else 
			_pos -= cycles * pose->frames.size();
	} 
}

void AnimatedObject::render(sdlx::Surface &surface, const int x, const int y) {
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
		LOG_WARN(("%s: event '%s' frame %d is out of range.", classname.c_str(), _events.front().name.c_str(), frame));
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

const std::string AnimatedObject::getState() const {
	static std::string empty;
	if (_events.empty())
		return empty;
	return _events.front().name;
}

void AnimatedObject::serialize(mrt::Serializator &s) const {
	Object::serialize(s);
	
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

void AnimatedObject::deserialize(const mrt::Serializator &s) {
	Object::deserialize(s);

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
