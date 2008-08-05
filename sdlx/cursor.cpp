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

#include "sdlx/cursor.h"
#include "sdlx.h"

const bool sdlx::Cursor::enabled() {
    return SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE;
}
void sdlx::Cursor::Disable() {
    SDL_ShowCursor(SDL_DISABLE);
}
void sdlx::Cursor::Enable() {
    SDL_ShowCursor(SDL_ENABLE);
}

void sdlx::Cursor::set_position(const int x, const int y) {
    SDL_WarpMouse(x, y);
}
void sdlx::Cursor::get_position(int &x, int &y) {
	SDL_GetMouseState(&x, &y);
}
