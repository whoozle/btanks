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

#include "sdlx/ttf.h"
#include <stdlib.h>
#include "sdlx/sdl_ex.h"
#include "sdlx/surface.h"


using namespace sdlx;

void TTF::init() {
	TTF_Init();
//	atexit(TTF_Quit); leads to various crashes at singleton dtors.
}

TTF::TTF() : _font(NULL) {}
TTF::~TTF() { close(); }
	
void TTF::open(const std::string &fname, const int psize) {
	_font = TTF_OpenFont(fname.c_str(), psize);
	if (_font == NULL)
		throw_sdl(("TTF_OpenFont"));
}

void TTF::renderBlended(sdlx::Surface &result, const std::string &text, const SDL_Color &fg) {
	SDL_Surface *r = TTF_RenderUTF8_Blended(_font, text.c_str(), fg);
	if (r == NULL)
		throw_sdl(("TTF_RenderUTF8_Blended"));
	result.assign(r);
}

void TTF::close() {
	if (_font == NULL) 
		return;
	
	TTF_CloseFont(_font);
	_font = NULL;
}
