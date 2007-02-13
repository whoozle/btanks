#include "font.h"
#include "surface.h"
#include "sdl_ex.h"

/* sdlx - c++ wrapper for libSDL
 * Copyright (C) 2005-2007 Vladimir Menshakov
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


using namespace sdlx;

Font::Font() : _type(), _surface(NULL){}

Font::~Font() {
	clear();
}

void Font::clear() {
	if (_surface == NULL) 
		return;
	delete _surface;
	_surface = NULL;
}
	
void Font::load(const std::string &file, const Type type, const bool alpha) {
	clear();
	_type = type;
	_surface = new sdlx::Surface;
	_surface->loadImage(file);
	_surface->convertAlpha();
	if (!alpha)
		_surface->setAlpha(0, 0);
}

const int Font::getHeight() const {
	return _surface->getHeight();
}

const int Font::render(sdlx::Surface &window, const int x, const int y, const std::string &str) const {
	int fw, fh;
	fw = fh = _surface->getHeight();

	for(unsigned i = 0; i < str.size(); ++i) {
		int c = str[i];
		
		switch(_type) {
		case Ascii:
			c -= 32;
			if (c < 0) 
				continue;
		break;
		case AZ09:
			c -= '0';
			if (c > 9)
				c -= 7;
			if (c < 0 || c > 36) 	
				continue;
		break;
		}
		
		sdlx::Rect src(c * fw, 0, fw, fh);
		
		window.copyFrom(*_surface, src, x + i * fw, y);
	}
	return str.size() * fw;
}

const int Font::render(sdlx::Surface &window, const std::string &str) const {
	if (str.empty())
		throw_ex(("in method render(new-surface, text), text must be non-empty"));
	int fw, fh;
	fw = fh = _surface->getHeight();
	
	window.createRGB(fw * str.size(), fw, 32, SDL_SRCALPHA);
	return render(window, 0, 0, str);
}

