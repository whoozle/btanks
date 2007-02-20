#include "container.h"

void Container::tick(const float dt) {
	for(ControlList::iterator i = _controls.begin(); i != _controls.end(); ++i) {
		i->second->tick(dt);
	}
}


void Container::render(sdlx::Surface &surface, const int x, const int y) {
	for(ControlList::iterator i = _controls.begin(); i != _controls.end(); ++i) {
		const sdlx::Rect &dst = i->first;
		i->second->render(surface, x + dst.x, y + dst.y);
	}
}

bool Container::onKey(const SDL_keysym sym) {
	for(ControlList::iterator i = _controls.begin(); i != _controls.end(); ++i) {
		if (i->second->onKey(sym))
			return true;
	}
	return false;
}

bool Container::onMouse(const int button, const bool pressed, const int x, const int y) {
	for(ControlList::iterator i = _controls.begin(); i != _controls.end(); ++i) {
		const sdlx::Rect &dst = i->first;
		if (dst.in(x, y) && i->second->onMouse(button, pressed, x - dst.x, y - dst.y))
			return true;
	}
	return false;
}

void Container::add(const sdlx::Rect &r, Control *ctrl) {
	_controls.push_back(ControlList::value_type(r, ctrl));
}

Container::~Container() {
	for(ControlList::iterator i = _controls.begin(); i != _controls.end(); ++i) {
		delete i->second;
	}
	_controls.clear();
}
