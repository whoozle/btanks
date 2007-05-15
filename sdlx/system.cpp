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

#include "sdlx/system.h"
#include "sdlx/sdlx.h"
#include "sdlx/sdl_ex.h"
#include <stdlib.h>

#include "mrt/logger.h"

using namespace sdlx;

void System::init(int system) {
	LOG_DEBUG(("calling SDL_init('%08x')", (unsigned)system));
	if (SDL_Init(system) == -1) 
		throw_sdl(("SDL_Init"));
}

void System::probeVideoMode() {
	LOG_DEBUG(("probing video info..."));
	char drv_name[256];
	if (SDL_VideoDriverName(drv_name, sizeof(drv_name)) == NULL)
		throw_sdl(("SDL_VideoDriverName"));
	LOG_DEBUG(("driver name: %s", drv_name));

	const SDL_VideoInfo * vinfo = SDL_GetVideoInfo();
	if (vinfo == NULL)
		throw_sdl(("SDL_GetVideoInfo()"));
	LOG_DEBUG(("hw_available: %u; wm_available: %u; blit_hw: %u; blit_hw_CC:%u; blit_hw_A:%u; blit_sw:%u; blit_sw_CC:%u; blit_sw_A: %u; blit_fill: %u; video_mem: %u", 
		vinfo->hw_available, vinfo->wm_available, vinfo->blit_hw, vinfo->blit_hw_CC, vinfo->blit_hw_A, vinfo->blit_sw, vinfo->blit_sw_CC, vinfo->blit_sw_A, vinfo->blit_fill, vinfo->video_mem ));
}

void System::deinit() {
	SDL_Quit();
}
