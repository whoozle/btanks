#include "container.h"
#include "mrt/logger.h"

void Container::tick(const float dt) {
	for(ControlList::iterator i = _controls.begin(); i != _controls.end(); ++i) {
		if (i->second->hidden())
			continue;
		
		i->second->tick(dt);
	}
}


void Container::render(sdlx::Surface &surface, const int x, const int y) {
	for(ControlList::iterator i = _controls.begin(); i != _controls.end(); ++i) {
		if (i->second->hidden())
			continue;

		const sdlx::Rect &dst = i->first;
		i->second->render(surface, x + dst.x, y + dst.y);
	}
}

bool Container::onKey(const SDL_keysym sym) {
	for(ControlList::reverse_iterator i = _controls.rbegin(); i != _controls.rend(); ++i) {
		if (i->second->hidden())
			continue;

		if (i->second->onKey(sym))
			return true;
	}
	return false;
}

bool Container::onMouse(const int button, const bool pressed, const int x, const int y) {
	//LOG_DEBUG(("%p: entering onMouse handler. (%d, %d)", (void *)this, x , y));
	for(ControlList::reverse_iterator i = _controls.rbegin(); i != _controls.rend(); ++i) {
		if (i->second->hidden())
			continue;

		const sdlx::Rect &dst = i->first;
		//LOG_DEBUG(("%p: checking control %p (%d, %d, %d, %d)", (void *)this, (void *)i->second, dst.x, dst.y, dst.w, dst.h));
		if (dst.in(x, y) && i->second->onMouse(button, pressed, x - dst.x, y - dst.y)) {
			//LOG_DEBUG(("%p: control %p returning true", (void *)this, (void *)i->second));
			return true;
		}
	}
	return false;
}

void Container::add(const sdlx::Rect &r, Control *ctrl) {
	_controls.push_back(ControlList::value_type(r, ctrl));
}

Container::~Container() {
	clear();
}

void Container::clear() {
	for(ControlList::iterator i = _controls.begin(); i != _controls.end(); ++i) {
		delete i->second;
	}
	_controls.clear();
}
