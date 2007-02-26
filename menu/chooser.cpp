#include "chooser.h"
#include "resource_manager.h"
#include "math/binary.h"
#include "sdlx/surface.h"

ImageChooser::ImageChooser(const std::string &surface, const int n) : _i(0), _n(n), index(0) {
	_surface = ResourceManager->loadSurface(surface);
	_left_right = ResourceManager->loadSurface("menu/left_right.png");
}

void ImageChooser::getSize(int &w, int &h) {
	w = _left_right->getWidth() + _surface->getWidth() / _n;
	h = math::max(_left_right->getHeight(), _surface->getHeight());
}

void ImageChooser::render(sdlx::Surface &surface, const int x, const int y) {
	Container::render(surface, x, y);
	int lrw = _left_right->getWidth() / 2;
	int lrh = _left_right->getHeight();
	
	int w, h;
	getSize(w, h);
	
	_left_area = sdlx::Rect(0, 0, lrw, lrh);
	_right_area = sdlx::Rect(w - lrw, 0, lrw, lrh);
	
	surface.copyFrom(*_left_right, sdlx::Rect(0, 0, lrw, lrh), x + _left_area.x, y + _left_area.y);
	
	surface.copyFrom(*_surface, 
		sdlx::Rect(_i * _surface->getWidth() / _n, 0, _surface->getWidth() / _n, _surface->getHeight()), 
		x + _left_area.x + lrw, y + (_left_area.h - _surface->getHeight())/ 2);
	
	surface.copyFrom(*_left_right, sdlx::Rect(lrw, 0, lrw, lrh), x +  _right_area.x, y + _right_area.y);
}
bool ImageChooser::onKey(const SDL_keysym sym) {
	return false;
}
bool ImageChooser::onMouse(const int button, const bool pressed, const int x, const int y) {
	if (_left_area.in(x, y)) {
		--_i;
		if (_i < 0)
			_i = _n - 1;
		return true;
	} else if (_right_area.in(x, y)) {
		++_i;
		if (_i >= _n)
			_i = 0;
		return true;
	} 
	return Container::onMouse(button, pressed, x, y);
}
