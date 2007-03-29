#include "slider.h"
#include "sdlx/surface.h"
#include "resource_manager.h"

Slider::Slider(const float value) : _n(10), _value(value) {
	if (value > 1.0f) 
		throw_ex(("slider accepts only values between 0 and 1 (inclusive)"));
	_tiles = ResourceManager->loadSurface("menu/slider.png");
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
	LOG_DEBUG(("%s", pressed?"+":"-"));
	return false;
}
