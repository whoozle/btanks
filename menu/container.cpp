#include "container.h"
#include "mrt/logger.h"
#include "sdlx/rect.h"

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

		const v2<int> &dst = i->first;
		i->second->render(surface, x + dst.x, y + dst.y);
	}
}

void Container::getSize(int &w, int &h) const {
	w = h = 0;
	for(ControlList::const_iterator i = _controls.begin(); i != _controls.end(); ++i) {
		int cw = -1, ch = -1; //for a broken controls
		i->second->getSize(cw, ch);
		assert(cw != -1 && ch != -1);

		int x2 = i->first.x + cw;
		int y2 = i->first.y + ch;

		if (x2 > w) 
			w = x2;

		if (y2 > h)
			h = y2;
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
		int bw, bh;
		i->second->getSize(bw, bh);
		
		const sdlx::Rect dst(i->first.x, i->first.y, bw, bh);
		//LOG_DEBUG(("%p: checking control %p (%d, %d, %d, %d)", (void *)this, (void *)i->second, dst.x, dst.y, dst.w, dst.h));
		if (dst.in(x, y) && i->second->onMouse(button, pressed, x - dst.x, y - dst.y)) {
			//LOG_DEBUG(("%p: control %p returning true", (void *)this, (void *)i->second));
			return true;
		}
	}
	return false;
}

void Container::add(const int x, const int y, Control *ctrl) {
	_controls.push_back(ControlList::value_type(v2<int>(x, y), ctrl));
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
