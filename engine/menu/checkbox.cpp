#include "checkbox.h"
#include "resource_manager.h"
#include "sdlx/surface.h"

Checkbox::Checkbox(const bool state) : 
	_state(state) , _checkbox(ResourceManager->load_surface("menu/checkbox.png")) {
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


void Checkbox::render(sdlx::Surface &surface, const int x, const int y) const {
	int tw = _checkbox->get_width() / 2, th = _checkbox->get_height();
	if (_state) {
		sdlx::Rect fg(tw, 0, _checkbox->get_width() - tw, th);
		surface.blit(*_checkbox, fg, x, y);
	} else {
		sdlx::Rect bg(0, 0, tw, th);
		surface.blit(*_checkbox, bg, x, y);
	}
}

void Checkbox::get_size(int &w, int &h) const {
	w = _checkbox->get_width() / 2;
	h = _checkbox->get_height();
}
