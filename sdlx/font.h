#ifndef __SDLX_FONT_H__
#define __SDLX_FONT_H__

/* sdlx - c++ wrapper for libSDL
 * Copyright (C) 2005-2007 Vladimir Menshakov
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include <string>
#include <vector>
#include <functional>
#include <map>
#include "export_sdlx.h"

namespace mrt {
	class Chunk;
}

namespace sdlx {

class Surface;
class SDLXAPI Font {
public:
	enum Type { Undefined, AZ09, Ascii };
	enum Align { Center, Left, Right };
	Font();
	~Font();
	
	void add_page(const unsigned base, const mrt::Chunk &data, const bool alpha = true);
	
	void load(const mrt::Chunk &data, const Type type, const bool alpha = true);
	void load(const std::string &file, const Type type, const bool alpha = true);
	const int get_height() const;
	const int get_width() const; //fixme! returns height ;)

	const int render(sdlx::Surface *window, int x, int y, const std::string &str) const;
	inline const int render(sdlx::Surface &window, int x, int y, const std::string &str) const {
		return render(&window, x, y, str);
	}
	
	//extend it with maximum_width argument and spacing
	//window == NULL, output in w/h
	//window != NULL: you must provide w/h (align == Center / Right)
	void render_multiline(int &w, int &h, sdlx::Surface *window, int x, int y, const std::string &str, Align align = Center) const;

	//frees window!
	const int render(sdlx::Surface &window, const std::string &str) const;
	void clear();

private:
	static const unsigned to_upper(const unsigned page, const unsigned c);

	Type _type;
	
	Font(const Font &);
	const Font& operator=(const Font &);
	
	struct Page {
		Page(bool alpha) : width_map(), surface(NULL), alpha(alpha) {}
		std::vector<std::pair<int, int> > width_map;
		sdlx::Surface *surface;
		bool alpha;
	};
	typedef std::map<const unsigned int, Page, std::greater<const unsigned int> > Pages;
	Pages _pages;
};

}

#endif

