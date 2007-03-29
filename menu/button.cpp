#include "button.h"
#include "sdlx/font.h"
#include "resource_manager.h"

Button::Button(const std::string &font, const std::string &label) : _font(ResourceManager->loadFont(font, true)), _label(label) {
	_w = _font->render(NULL, 0, 0, label);
	_background.init("menu/background_box.png", _w + 32, _font->getHeight() + 16);
}


void Button::render(sdlx::Surface& surface, int x, int y) {
	_background.render(surface, x, y);
	
	_font->render(surface, x + (_background.w - _w) / 2, y + (_background.h - _font->getHeight()) / 2, _label);
}

bool Button::onMouse(const int button, const bool pressed, const int x, const int y) {
	if (!pressed) 
		return false;
	_changed = true;
	return true;
}

void Button::getSize(int &w, int &h) const {
	w = _background.w;
	h = _background.h;
}

