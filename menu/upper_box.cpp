#include "upper_box.h"

void UpperBox::init(int w, int h, const bool server) {
	_server = server;
	if (_server) {
		
	}
	Box::init("menu/background_box.png", w, h);
}

void UpperBox::render(sdlx::Surface &surface, const int x, const int y) {
	Box::render(surface, x, y);
}
