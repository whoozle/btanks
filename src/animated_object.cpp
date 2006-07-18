#include "animated_object.h"
#include "sdlx/surface.h"
#include "mrt/logger.h"
#include "animation_model.h"

AnimatedObject::AnimatedObject(AnimationModel *model, sdlx::Surface *surface, const int tile_w, const int tile_h) {
	_model = model;
	_surface = surface;
	_tw = tile_w; _th = tile_h;
	_direction= 0;
	_pos = 0;
}

void AnimatedObject::setDirection(const int dir) {
	_direction = dir;
}

const int AnimatedObject::getDirection() const {
	return _direction;
}

void AnimatedObject::play(const std::string &id, const bool repeat) {
	if (_events.empty())
		_pos = 0;
	const Pose *pose = _model->getPose(id);
	if (pose != NULL)
		_events.push_back(Event(id, repeat, pose));
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
	//LOG_DEBUG(("event: %s, pos = %f", event.name.c_str(), _pos));
	const Pose * pose = event.pose;
	
	if (pose == NULL) {
		cancel();
		return;
	}
	
	_pos += dt * pose->speed;
	int cycles = ((int)_pos / pose->frames.size());
	if (cycles && !event.repeat) {
		cancel();
	} else {
		_pos -= cycles * pose->frames.size();
	}
}

void AnimatedObject::render(sdlx::Surface &surface, const int x, const int y) {
	if (_events.empty()) 
		return;
	unsigned frame = (unsigned)_pos;
	frame = _events.front().pose->frames[frame];
	
	if (frame * _th >= (unsigned)_surface->getHeight()) {
		LOG_WARN(("frame %u is out of range.", frame));
		return;
	}
	
	sdlx::Rect src(_direction * _tw, frame * _th, _tw, _th);
	surface.copyFrom(*_surface, src, x, y);
}

const std::string AnimatedObject::getState() const {
	static std::string empty;
	if (_events.empty())
		return empty;
	return _events.front().name;
}
