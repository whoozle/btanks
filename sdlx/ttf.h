#ifndef ___SDLX_TTF_H__
#define ___SDLX_TTF_H__

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

#include "SDL/SDL_ttf.h"
#include <string>

namespace sdlx {
class Surface;
class TTF {
public: 
	static void init();
	
	TTF();
	~TTF();
	
	void open(const std::string &fname, const int psize);
	void renderBlended(sdlx::Surface &result, const std::string &text, const SDL_Color &fg);
	void close();
private:
	TTF_Font * _font;
};

}

#endif

