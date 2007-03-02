#include "prompt.h"
#include "text_control.h"

Prompt::Prompt(const int w, const int h, TextControl * text) {
	_background.init("menu/background_box.png", "menu/highlight_medium.png", w, h);
	int mx, my;
	_background.getMargins(mx, my);
	add(sdlx::Rect(mx, my, w - mx * 2, h - my * 2), _text = text);
}

void Prompt::set(const std::string &value) {
	_text->set(value);
}

const std::string &Prompt::get() const {
	return _text->get();
}

void Prompt::tick(const float dt) {
	Container::tick(dt);
	if (_text->changed()) {
		_text->reset();
		_changed = true;
	}
}


void Prompt::getSize(int &w , int &h) const {
	w = _background.w; h = _background.h;
}


void Prompt::render(sdlx::Surface& surface, int x, int y) {
	_background.render(surface, x, y);
	Container::render(surface, x, y);
}
