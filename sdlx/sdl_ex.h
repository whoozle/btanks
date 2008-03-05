#ifndef __SDL_CXX_LAYER_EXCEPTION_H__
#define __SDL_CXX_LAYER_EXCEPTION_H__

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
#include <exception>
#include "sdlx/export_sdlx.h"
#include "mrt/fmt.h"
#include "mrt/exception.h"

namespace sdlx {
DERIVE_EXCEPTION(SDLXAPI, Exception);
}

#define throw_sdl(s) throw_generic(sdlx::Exception, s);

#endif
