
/* Battle Tanks Game
 * Copyright (C) 2006 Battle Tanks team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "menuitem.h"
#include "sdlx/ttf.h"
#include "mrt/logger.h"

MenuItem::MenuItem(sdlx::TTF &font, const std::string &name, const std::string &type, const std::string &text, const std::string &value) : 
	name(name), type(type), 
	_font(font), _color(sdlx::Color(255, 255, 255)), 
	_text(text), _value(value) {
	render(font);
}

const std::string MenuItem::getValue() const {
	return _value;
}


void MenuItem::render(sdlx::TTF &font) {
	_normal.free();
	_inversed.free();

	font.renderBlended(_normal, _text, _color);
	_normal.convertAlpha();
	_normal.convertToHardware();
	//LOG_DEBUG(("normal  : %dx%d:%d (%d)", _normal.getWidth(), _normal.getHeight(), _normal.getBPP(), _normal.getSDLSurface()->format->BytesPerPixel));

	_inversed.createRGB(_normal.getWidth(), _normal.getHeight(), _normal.getBPP(), SDL_SWSURFACE);
	_inversed.convertAlpha();
	_inversed.convertToHardware();
	//_inversed.setAlpha(255);
		
	//LOG_DEBUG(("inversed: %dx%d:%d", _inversed.getWidth(), _inversed.getHeight(), _inversed.getBPP()));
	_normal.lock();
	_inversed.lock();
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
	_normal.unlock();
	_inversed.unlock();
}
	
void MenuItem::render(sdlx::Surface &dst, const int x, const int y, const bool inverse) {
	dst.copyFrom(inverse?_inversed:_normal, x, y);
}

void MenuItem::getSize(int &w, int &h) const {
	w = _normal.getWidth();
	h = _normal.getHeight();
}
