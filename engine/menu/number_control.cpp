#include "number_control.h"
#include "sdlx/font.h"
#include "sdlx/surface.h"
#include "math/binary.h"
#include "resource_manager.h"


NumberControl::NumberControl(const std::string &font, const int min, const int max, const int step) : 
	min(min), max(max), step(step), value(min), 
	mouse_pressed(0), mouse_button(0), direction(false), 
	_number(ResourceManager->load_surface("menu/number.png")), 
	_font(ResourceManager->loadFont(font, true)), 
	r_up(0, 0, _number->get_width(), _number->get_height() / 2), 
	r_down(0, _number->get_height() / 2, _number->get_width(), _number->get_height() - _number->get_height() / 2) {}

const int NumberControl::get() const {
	const_cast<NumberControl *>(this)->validate();
	return value;
}

void NumberControl::validate() {
	if (value < min)
		value = min;
	if (value > max)
		value = max;
}

void NumberControl::set(const int v) {
	if (v > max || v < min)
		return;
	value = min + (v - min) / step * step;
	validate();
}

void NumberControl::render(sdlx::Surface &surface, const int x, const int y) const {
	surface.blit(*_number, x, y);
	_font->render(surface, x + _number->get_width(), y + _number->get_height() - _font->get_height(), mrt::format_string(min < 0?"%+d":"%d", value));
}

void NumberControl::get_size(int &w, int &h) const {
	w =	_font->render(NULL, 0, 0, mrt::format_string(min < 0?"%+d":"%d", value)) + _number->get_width();
	h = math::max(_number->get_height(), _font->get_height());
}

void NumberControl::up(const int v) {
	value += v * step;
	validate();
}

void NumberControl::down(const int v) {
	value -= v * step;
	validate();
}

bool NumberControl::onKey(const SDL_keysym sym) {
	switch(sym.sym) {
		case SDLK_UP: up(); return true;
		case SDLK_DOWN: down(); return true;
		case SDLK_PAGEUP: up(10); return true;
		case SDLK_PAGEDOWN: down(10); return true;

		case SDLK_BACKSPACE: value /= 10;
			return true;
		default: 
			if (sym.unicode >= '0' && sym.unicode <= '9') {
				value = sym.unicode - '0' + 10 * value;
				if (value > max)
					value = max;
				return true;
			}
		
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
			up(mouse_button == SDL_BUTTON_RIGHT? 10 * n: n);
		} else {
			down(mouse_button == SDL_BUTTON_RIGHT? 10 * n: n);
		}
	}

}

void NumberControl::setMinMax(const int m1, const int m2) { 
	LOG_DEBUG(("setting min: %d, max: %d", m1, m2));
	min = m1; 
	max = m2; 
	validate(); 
}
