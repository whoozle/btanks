#include "slider.h"
#include "sdlx/surface.h"
#include "resource_manager.h"
#include "game.h"
#include "sdlx/sdlx.h"

Slider::Slider(const float value) : _n(10), _value(value), _grab(false) {
	if (value > 1.0f) 
		throw_ex(("slider accepts only values between 0 and 1 (inclusive)"));
	_tiles = ResourceManager->loadSurface("menu/slider.png");

	Game->mouse_motion_signal.connect(sigc::mem_fun(this, &Slider::onMouseMotion));	
}


void Slider::render(sdlx::Surface &surface, const int x, const int y) {
	int w = _tiles->getWidth() / 2, h = _tiles->getHeight();
	sdlx::Rect bound(0, 0, w, h), pointer(w, 0, w, h);
	for(int i = 0; i < _n; ++i) 
		surface.copyFrom(*_tiles, bound, x + i * w, y);
	int xp = x + (int)(_value * _n * w) - w / 2;
	surface.copyFrom(*_tiles, pointer, xp, y);
}

void Slider::getSize(int &w, int &h) const {
	w = (_tiles->getWidth() / 2) * _n;
	h = _tiles->getHeight();
}

bool Slider::onMouse(const int button, const bool pressed, const int x, const int y) {
	if (!pressed && _grab) {
		_grab = false;
		return true;
	}
	if (pressed && !_grab) {
		_grab = true;
		_grab_state = SDL_GetMouseState(NULL, NULL);
	}	
	return false;
}

bool Slider::onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel) {
	if (!_grab) 	
		return false;
	if (state != _grab_state) {
		_grab = false;
		return true;
	}
	int w = _tiles->getWidth() / 2;
	_value += 1.0 * xrel / w / _n;
	if (_value < 0)
		_value = 0;
	else if (_value > 1)
		_value = 1;
	//LOG_DEBUG(("tracking mouse: %d %d %d %d", x, y, xrel, yrel));
	
	return true;
}
