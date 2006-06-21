#include "menuitem.h"
#include "sdlx/ttf.h"
#include "mrt/logger.h"

MenuItem::MenuItem(sdlx::TTF &font, const std::string &name, const std::string &type, const std::string &value) : 
	name(name), type(type), _font(font), _color(sdlx::Color(255, 255, 255)), _value(value) {
	render(font);
}

void MenuItem::render(sdlx::TTF &font) {
	_normal.free();
	_inversed.free();

	font.renderBlended(_normal, _value, _color);
	LOG_DEBUG(("normal  : %dx%d:%d (%d)", _normal.getWidth(), _normal.getHeight(), _normal.getBPP(), _normal.getSDLSurface()->format->BytesPerPixel));

	_inversed.createRGB(_normal.getWidth(), _normal.getHeight(), _normal.getBPP(), SDL_SWSURFACE);
	_inversed.convertAlpha();
	//_inversed.setAlpha(255);
		
	LOG_DEBUG(("inversed: %dx%d:%d", _inversed.getWidth(), _inversed.getHeight(), _inversed.getBPP()));

	int w = _normal.getWidth();
	int h = _normal.getHeight();
	for(int y = 0; y < h; ++y) 
		for(int x = 0; x < w; ++x) {
			Uint32 c = _normal.getPixel(x, y);
			//LOG_DEBUG(("%08x ", (unsigned )c));
			Uint8 r, g, b, a;
			_normal.getRGBA(c, r, g, b, a);
			//LOG_DEBUG(("%02x %02x %02x %02x", a, r, g, b));
			if (r == 0 && g == 0 && b == 0 || a == 0) {
				r = 0; g = 0; b = 128; a = 255;
			}
			//LOG_DEBUG(("%02x %02x %02x %02x", a, r, g, b));
			c = _inversed.mapRGBA(r, g, b, a);
			//LOG_DEBUG(("%08x ", (unsigned )c));
			_inversed.putPixel(x, y, c);
		}
}
	
void MenuItem::render(sdlx::Surface &dst, const int x, const int y, const bool inverse) {
	dst.copyFrom(inverse?_inversed:_normal, x, y);
}

void MenuItem::getSize(int &w, int &h) const {
	w = _normal.getWidth();
	h = _normal.getHeight();
}
