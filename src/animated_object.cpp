#include "animated_object.h"
#include "sdlx/surface.h"
#include "mrt/exception.h"
#include "mrt/logger.h"
#include "animation_model.h"
#include "resource_manager.h"

AnimatedObject::AnimatedObject(const std::string &classname) : 
	Object(classname),  _model(0), _surface(0), _direction_idx(0), _pos(0)  {}

void AnimatedObject::init(const std::string &model, sdlx::Surface *surface, const int tile_w, const int tile_h) {
	_model_name = model;
	_model = ResourceManager->getAnimationModel(model);
	_events.clear();
	_surface = surface;
	size.x = _tw = tile_w; size.y = _th = tile_h;
	_direction_idx = 0;
	_pos = 0;
}

void AnimatedObject::init(const AnimatedObject &o) {
	_events.clear();
	std::string c = classname; //fixme: rework code in resource manager to do not workaround it.
	
	//LOG_DEBUG(("classname: %s, model_name: %s", classname.c_str(), _model_name.c_str()));
	//LOG_DEBUG(("o.classname: %s, o.model_name: %s", o.classname.c_str(), o._model_name.c_str()));
	*this = o;
	
	classname = c;
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
	const Pose *pose = _model->getPose(id);
	if (pose != NULL)
		_events.push_back(Event(id, repeat, pose));
}

void AnimatedObject::playNow(const std::string &id) {
	const Pose *pose = _model->getPose(id);
	if (pose == NULL) 
		return;
	_pos = 0;
	_events.push_front(Event(id, false, pose));
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
	const Pose * pose = event.pose;
	
	if (pose == NULL) {
		cancel();
		return;
	}
	
	_pos += dt * pose->speed;
	//LOG_DEBUG(("%s: _pos: %f", classname.c_str(), _pos));
	int cycles = ((int)_pos / pose->frames.size());
	if (cycles && !event.repeat) {
		cancel();
	} else {
		_pos -= cycles * pose->frames.size();
	}
}

void AnimatedObject::render(sdlx::Surface &surface, const int x, const int y) {
	if (_events.empty()) {
		if (!isDead())
			LOG_WARN(("%s: no animation played", classname.c_str()));
		return;
	}
	int frame = (int)_pos;
	if (frame < 0 || frame >= (int)_events.front().pose->frames.size()) {
		LOG_WARN(("%s: event '%s' frame %d is out of range.", classname.c_str(), _events.front().name.c_str(), frame));
		return;		
	}
	frame = _events.front().pose->frames[frame];
	
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
	s.add(_model_name);
}
void AnimatedObject::deserialize(const mrt::Serializator &s) {
	Object::deserialize(s);
}
