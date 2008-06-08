#ifndef __sdlx_CURSOR_H__
#define __sdlx_CURSOR_H__

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


#include "export_sdlx.h"

namespace sdlx {
    class SDLXAPI Cursor {
	public:
	static const bool enabled();
	static void Disable();
	static void Enable();
	static void set_position(const int x, const int y);
	static void get_position(int &x, int &y);
    };
}

#endif
