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

#include "font.h"
#include "surface.h"
#include "sdl_ex.h"
#include <assert.h>
#include <ctype.h>


using namespace sdlx;


const unsigned Font::toUpper(const unsigned page, const unsigned c) {
	//fixme: 
	//LOG_DEBUG(("toUpper(%04x, %x)", page, c));
	switch(page) {
	case 0x0020: 
		if (c > 0x40 && c <= 0x5a) 
			return c + 0x20;
		return c;
	case 0x00a0: 
		if (c >= 0x40)
			return c - 0x20;
		return c;
	case 0x0400:
		if (c >= 0x30 && c < 0x50) 
			return c - 0x20;
		if (c >= 0x50 && c < 0x60)
			return c - 0x50;
		return c;
	default:
		return c;
	}
}

Font::Font() : _type(Undefined) {}

Font::~Font() {
	clear();
}

void Font::clear() {
	for(Pages::iterator i = _pages.begin(); i != _pages.end(); ++i) {
		delete i->second.surface;
	}
	_pages.clear();
}

void Font::addPage(const unsigned base, const std::string &file, const bool alpha) {
	Page page;

	page.surface = new sdlx::Surface;
	page.surface->loadImage(file);
	page.surface->convertAlpha();
	
	if (!alpha)
		page.surface->setAlpha(0, 0);
	
	//scanning pixel width;
	int h = page.surface->getHeight();
	int w = h;
	int n = (page.surface->getWidth() - 1) / w + 1;
	
	page.width_map.resize(n);
	
	for(int c = 0; c < n; ++c) {
		page.width_map[c].first = w;
		page.width_map[c].second = 0;
		
		for(int y = 0; y < h; ++y) {
			int x1, x2;
			
			for(x1 = 0; x1 < w; ++x1) {
				Uint32 p = page.surface->getPixel(x1 + c * w, y);
				Uint8 r, g, b, a;
				page.surface->getRGBA(p, r, g, b, a);
				if (a > 128) {
					//LOG_DEBUG(("line %d:%d, break on %d %d %d %d", y, x1, r, g, b, a));
					break;
				}
			}
			
			for(x2 = w - 1; x2 >= 0; --x2) {
				Uint32 p = page.surface->getPixel(x2 + c * w, y);
				Uint8 r, g, b, a;
				page.surface->getRGBA(p, r, g, b, a);
				if (a > 128) {
					//LOG_DEBUG(("line %d:%d, break on %d %d %d %d", y, x2, r, g, b, a));
					break;
				}
			}
			
			if (x1 < page.width_map[c].first)
				page.width_map[c].first = x1;
			if (x2 > page.width_map[c].second)
				page.width_map[c].second = x2;
		}
		
		if (page.width_map[c].first > page.width_map[c].second) {
			page.width_map[c].first = 0;
			page.width_map[c].second = w / 3;
		} 
		
		//LOG_DEBUG(("%s: char: %d, x1: %d, x2: %d", file.c_str(), c, _width_map[c].first, _width_map[c].second));
	}
	_pages[base] = page;
}
	
void Font::load(const std::string &file, const Type type, const bool alpha) {
	clear();
	_type = type;
	addPage(0x20, file, alpha);
}

const int Font::getHeight() const {
	if (_pages.empty())
		throw_ex(("font was not loaded"));
	return _pages.begin()->second.surface->getHeight();
}

const int Font::getWidth() const {
	return getHeight();
}

const int Font::render(sdlx::Surface &window, const int x, const int y, const std::string &str) const {
	return render(&window, x, y, str);
}

#include <deque>

#define CHECK_SIZE(i) if ((i) >= str.size()) { \
	tokens.push_back('?');\
	break;\
}


const int Font::render(sdlx::Surface *window, const int x, const int y, const std::string &str) const {
	int w = 0;
	
	std::deque<unsigned> tokens;
	
	for(size_t i = 0; i < str.size(); ) {
		unsigned c = (unsigned)str[i++];
		if (c < 0x80) {
			tokens.push_back(c);
		} else if ((c & 0xc0) == 0x80) {
			//tokens.push_back('?'); //unsupported sequence
		} else if ((c & 0xe0) == 0xc0) {
			CHECK_SIZE(i);
			
			unsigned b2 = str[i++];
			if ((b2 & 0xc0) != 0x80) {
				tokens.push_back('?');
				continue;
			}
			tokens.push_back(((c & 0x1f) << 6) | (b2 & 0x3f));
		} else if ((c & 0xf0) == 0xe0) {
			CHECK_SIZE(i);
			unsigned b2 = str[i++];
			CHECK_SIZE(i);
			unsigned b3 = str[i++];

			if ((b2 & 0xc0) != 0x80 || (b3 & 0xc0) != 0x80) {
				tokens.push_back('?');
				continue;				
			}
			
			tokens.push_back(((c & 0x0f) << 12) | ((b2 & 0x3f) << 6) | (b3 & 0x3f));
		} else if ((c & 0xf8) == 0xf0) {
			CHECK_SIZE(i);
			unsigned b2 = str[i++];
			CHECK_SIZE(i);
			unsigned b3 = str[i++];
			CHECK_SIZE(i);
			unsigned b4 = str[i++];
			if ((b2 & 0xc0) != 0x80 || (b3 & 0xc0) != 0x80 || (b4 & 0xc0) != 0x80) {
				tokens.push_back('?');
				continue;
			}
			tokens.push_back(((c & 0x07) << 18) | ((b2 & 0x3f) << 12) | ((b3 & 0x3f) << 6) | (b4 & 0x3f));
		} else {
			tokens.push_back('*');
		}
	}	

	for(std::deque<unsigned>::const_iterator i = tokens.begin(); i != tokens.end(); ++i) {
		unsigned c = *i;
		//LOG_DEBUG(("token: '%c' %x %s", (c >= 0x20 && c < 0x80)?c:'.', c, c >= 0x80?"<<<<<<<<<<":""));
		//Pages::const_iterator page_i = _pages.lower_bound(c);
		//if (page_i == _pages.end())
		//	continue;
		Pages::const_iterator page_i;
		for(page_i = _pages.begin(); page_i != _pages.end() && c < page_i->first; ++page_i);

		unsigned page_base = 0x20;
		if (page_i == _pages.end()) {
			c = '?';
		} else {
			page_base = page_i->first;
		}
		const Page & page = (page_i != _pages.end())? page_i->second: _pages.rbegin()->second;
		
		//LOG_DEBUG(("page: %04x", page_i->first));
		//LOG_DEBUG(("token: %08x base: U+%08x, offset: %08x", c, page_base, c - page_base));

		if (c < page_base) {
			c = '?';
			page_base = 0x20;
		}

		int fw, fh;
		fw = fh = page.surface->getHeight();
		
		switch(_type) {
		case Ascii:

			if (c < 0x80 && (c - page_base) * fw >= (unsigned)page.surface->getWidth())
				c = toupper(c);


			c -= page_base;
	
			if (c * fw >= (unsigned)page.surface->getWidth())
				c = toUpper(page_base, c); //last try, try upper
			
		break;
		case Undefined: 
			throw_ex(("font without type"));
		case AZ09:
			throw_ex(("rewrite me"));
		}
		
		int x1 = 0, x2 = fw - 1;
		int spacing = fw / 8 - 1;
		if (spacing > 4) 
			spacing = 4;
		//const int spacing = 2;

		if (_type == AZ09 && c == ' ') {
			w += fw / 3 + spacing;
			continue;
		}
		
		if (c < page.width_map.size()) {
			//LOG_DEBUG(("char '%c' (code: %d), %d<->%d", str[i], c, x1, x2));
			x1 = page.width_map[c].first;
			x2 = page.width_map[c].second;
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
		
			window->copyFrom(*page.surface, src, x + w, y);
		}
		w += spacing;
		
		w += x2 - x1 + 1;
	}
	return (w > 0)?w:1;
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

