#include "number_control.h"
#include "sdlx/font.h"
#include "sdlx/surface.h"
#include "math/binary.h"
#include "resource_manager.h"

NumberControl::NumberControl(const std::string &font, const int min, const int max, const int step) : 
	min(min), max(max), step(step), value(min), 
	mouse_pressed(0), mouse_button(0), direction(false), 
	_number(ResourceManager->loadSurface("menu/number.png")), 
	_font(ResourceManager->loadFont(font, true)), 
	r_up(0, 0, _number->getWidth(), _number->getHeight() / 2), 
	r_down(0, _number->getHeight() / 2, _number->getWidth(), _number->getHeight() - _number->getHeight() / 2) {}

void NumberControl::set(const int v) {
	if (v > max || v < min)
		return;
	value = (v - min) / step * step;
}

void NumberControl::render(sdlx::Surface &surface, const int x, const int y) {
	surface.copyFrom(*_number, x, y);
	_font->render(surface, x + _number->getWidth(), y + _number->getHeight() - _font->getHeight(), mrt::formatString("%d", value));
}

void NumberControl::getSize(int &w, int &h) const {
	w =	_font->render(NULL, 0, 0, "00000") + _number->getWidth();
	h = math::max(_number->getHeight(), _font->getHeight());
}

void NumberControl::up(const int v) {
	if (value + v <= max)
		value += v;
}

void NumberControl::down(const int v) {
	if (value - v >= min)
		value -= v;
}

bool NumberControl::onKey(const SDL_keysym sym) {
	switch(sym.sym) {
		case SDLK_UP: up(); return true;
		case SDLK_DOWN: down(); return true;
		case SDLK_PAGEUP: up(step * 10); return true;
		case SDLK_PAGEDOWN: down(step * 10); return true;
		default: 
			return false;
	}
}

bool NumberControl::onMouse(const int button, const bool pressed, const int x, const int y) {
	if (r_up.in(x, y) && pressed) {
		up(button == SDL_BUTTON_RIGHT? 10: 1);
		mouse_button = button;
		direction = true;
		mouse_pressed = 0.0;
		return true;
	}
	if (r_down.in(x, y) && pressed) {
		down(button == SDL_BUTTON_RIGHT? 10: 1);
		mouse_button = button;
		direction = false;
		mouse_pressed = 0.0;
		return true;
	}
	if (!pressed) {
		mouse_pressed = 0;
		mouse_button = 0;
	}
	return false;
}

void NumberControl::tick(const float dt) {
	Control::tick(dt);
	if (mouse_button != 0) {
		mouse_pressed += dt;
		const float guard = 0.5f;
		if (mouse_pressed < guard)
			return;
		
		const int speed = 20;
		int n = (int)((mouse_pressed - guard) * speed); //10 - speed
		mouse_pressed -= 1.0f * n / speed;

		//don't blame me, i am an indian programmer :)))
		if (direction) {
			for(int i = 0; i < n; ++i)
				up(mouse_button == SDL_BUTTON_RIGHT? 10: 1);
		} else {
			for(int i = 0; i < n; ++i)
				down(mouse_button == SDL_BUTTON_RIGHT? 10: 1);
		}
	}

}
