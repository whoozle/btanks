#include "font.h"
#include "surface.h"
#include "sdl_ex.h"
#include <assert.h>

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
	
	//scanning pixel width;
	int h = _surface->getHeight();
	int w = h;
	int n = (_surface->getWidth() - 1) / w + 1;
	
	_width_map.resize(n);
	
	for(int c = 0; c < n; ++c) {
		_width_map[c].first = w - 1 ;
		_width_map[c].second = 0;
		
		for(int y = 0; y < h; ++y) {
			int x1, x2;
			
			for(x1 = 0; x1 < w; ++x1) {
				Uint32 p = _surface->getPixel(x1 + c * w, y);
				Uint8 r, g, b, a;
				_surface->getRGBA(p, r, g, b, a);
				if (a > 128) {
					//LOG_DEBUG(("line %d:%d, break on %d %d %d %d", y, x1, r, g, b, a));
					break;
				}
			}
			
			for(x2 = w - 1; x2 >= 0; --x2) {
				Uint32 p = _surface->getPixel(x2 + c * w, y);
				Uint8 r, g, b, a;
				_surface->getRGBA(p, r, g, b, a);
				if (a > 128) {
					//LOG_DEBUG(("line %d:%d, break on %d %d %d %d", y, x2, r, g, b, a));
					break;
				}
			}
			
			if (x1 < _width_map[c].first)
				_width_map[c].first = x1;
			if (x2 > _width_map[c].second)
				_width_map[c].second = x2;
		}
		
		if (_width_map[c].first > _width_map[c].second) {
			_width_map[c].first = 0;
			_width_map[c].second = w / 3;
		} 
		
		//LOG_DEBUG(("%s: char: %d, x1: %d, x2: %d", file.c_str(), c, _width_map[c].first, _width_map[c].second));
	}
}

const int Font::getHeight() const {
	return _surface->getHeight();
}

const int Font::getWidth() const {
	return getHeight();
}

const int Font::render(sdlx::Surface &window, const int x, const int y, const std::string &str) const {
	return render(&window, x, y, str);
}


const int Font::render(sdlx::Surface *window, const int x, const int y, const std::string &str) const {
	int fw, fh;
	fw = fh = _surface->getHeight();
	int w = 0;

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
		
		int x1 = 0, x2 = fw - 1;
		const int spacing = 1;
		
		if (c < (int)_width_map.size()) {
			x1 = _width_map[c].first;
			x2 = _width_map[c].second;
			/*
			if (x1 >= spacing) 
				x1 -= spacing;
			else x1 = 0;
			if (x2 < fw - spacing) 
				x2 += spacing;
			else x2 = fw - 1;
			*/
		}
		
		if (window != NULL) {
			sdlx::Rect src(c * fw + x1, 0, x2 - x1 + 1, fh);
		
			window->copyFrom(*_surface, src, x + w, y);
		}
		w += spacing;
		
		w += x2 - x1 + 1;
	}
	return w;
}

const int Font::render(sdlx::Surface &window, const std::string &str) const {
	if (str.empty())
		throw_ex(("in method render(new-surface, text), text must be non-empty"));

	int h = getHeight();
	int w = render(NULL, 0, 0, str);
	
	window.createRGB(w, h, 32, SDL_SRCALPHA);
	window.convertAlpha();
	return render(&window, 0, 0, str);
}

