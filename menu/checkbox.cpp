#include "checkbox.h"
#include "resource_manager.h"
#include "sdlx/surface.h"

Checkbox::Checkbox(const bool state) : 
	_state(state) , _checkbox(ResourceManager->loadSurface("menu/checkbox.png")) {
}

const bool Checkbox::get() const {
	return _state;
}

void Checkbox::set(const bool value) {
	_state = value;
}


bool Checkbox::onMouse(const int button, const bool pressed, const int x, const int y) {
	if (pressed) 
		return true;
	_state = !_state;
	invalidate(true);
	return true;
}


void Checkbox::render(sdlx::Surface &surface, const int x, const int y) {
	int tw = _checkbox->getWidth() / 2, th = _checkbox->getHeight();
	if (_state) {
		sdlx::Rect fg(tw, 0, _checkbox->getWidth() - tw, th);
		surface.copyFrom(*_checkbox, fg, x, y);
	} else {
		sdlx::Rect bg(0, 0, tw, th);
		surface.copyFrom(*_checkbox, bg, x, y);
	}
}

void Checkbox::getSize(int &w, int &h) const {
	w = _checkbox->getWidth() / 2;
	h = _checkbox->getHeight();
}
