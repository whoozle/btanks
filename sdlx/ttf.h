#ifndef ___SDLX_TTF_H__
#define ___SDLX_TTF_H__

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

#include <SDL_ttf.h>
#include <string>
#include "export_sdlx.h"

namespace sdlx {
class Surface;
class SDLXAPI TTF {
public: 
	static void init();
	
	TTF();
	~TTF();
	
	void open(const std::string &fname, const int psize);
	void renderBlended(sdlx::Surface &result, const std::string &text, const SDL_Color &fg);
	void close();
private:
	TTF(const TTF &);
	TTF & operator=(const TTF &);
	
	TTF_Font * _font;
};

}

#endif

