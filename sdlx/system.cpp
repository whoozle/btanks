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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "mrt/logger.h"

using namespace sdlx;

static void WIN_FlushMessageQueue()
{
	MSG  msg;
	while ( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) ) {
		if ( msg.message == WM_QUIT ) break;
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}
}

const bool System::acceleratedGL(const bool windowed) {
	bool accel = true;
TRY {
#ifdef _WINDOWS
	HWND hwnd = CreateWindow("SDL_app", "SDL_app", WS_POPUP | WS_DISABLED,
	                    0, 0, 10, 10,
	                    NULL, NULL, GetModuleHandle(NULL), NULL);
	if (hwnd == NULL)
		throw_ex(("CreateWindow failed"));
   	
	WIN_FlushMessageQueue();

	HDC hdc = GetDC(hwnd);

	PIXELFORMATDESCRIPTOR pfd;//, pfd2;
	memset(&pfd, 0, sizeof(pfd));
	
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = (windowed?PFD_DRAW_TO_WINDOW:0) | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_GENERIC_ACCELERATED;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;

	int pf = ChoosePixelFormat(hdc, &pfd);
	::DescribePixelFormat(hdc, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd); 
	
	LOG_DEBUG(("best pixel format: #%02d, bits: %02d, flags: %08x %s%s%s%s%s%s%s", 
		pf, pfd.cColorBits, pfd.dwFlags, 
		((pfd.dwFlags & PFD_SUPPORT_OPENGL) == PFD_SUPPORT_OPENGL)?"PFD_SUPPORT_OPENGL ":"", 
		((pfd.dwFlags & PFD_SUPPORT_GDI) == PFD_SUPPORT_GDI)?"PFD_SUPPORT_GDI ":"", 
		((pfd.dwFlags & PFD_GENERIC_FORMAT) == PFD_GENERIC_FORMAT)?"PFD_GENERIC_FORMAT ":"",
		((pfd.dwFlags & PFD_GENERIC_ACCELERATED) == PFD_GENERIC_ACCELERATED)?"PFD_GENERIC_ACCELERATED ":"",
		((pfd.dwFlags & PFD_SUPPORT_DIRECTDRAW) == PFD_SUPPORT_DIRECTDRAW)?"PFD_SUPPORT_DIRECTDRAW ":"",
		((pfd.dwFlags & PFD_DOUBLEBUFFER) == PFD_DOUBLEBUFFER)?"PFD_DOUBLEBUFFER ":"",
		((pfd.dwFlags & PFD_DRAW_TO_WINDOW) == PFD_DRAW_TO_WINDOW)?"PFD_DRAW_TO_WINDOW ":""
	));

	if (
		((pfd.dwFlags & PFD_SUPPORT_OPENGL) != PFD_SUPPORT_OPENGL) ||
		( 
			(((pfd.dwFlags & PFD_GENERIC_FORMAT) == PFD_GENERIC_FORMAT) &&
			((pfd.dwFlags & PFD_GENERIC_ACCELERATED) != PFD_GENERIC_ACCELERATED))
		)
	) {
		accel = false;
	}

/*
	pfd2 = pfd;
	pfd2.dwFlags = 0;

	int n = ::DescribePixelFormat( hdc, 1, sizeof(PIXELFORMATDESCRIPTOR), &pfd2); 
	//LOG_DEBUG(("pixelformats: %d", n));
	for(int i = 1; i <= n; ++i) {
		if (::DescribePixelFormat( hdc, i, sizeof(PIXELFORMATDESCRIPTOR), &pfd) == 0)
			continue;
			if (pfd.cColorBits < 24 || pfd.iPixelType != PFD_TYPE_RGBA)
				continue;
			LOG_DEBUG(("pixel format: #%02d, bits: %02d, flags: %08x %s%s%s%s%s%s%s", 
				i, pfd.cColorBits, pfd.dwFlags, 
				((pfd.dwFlags & PFD_SUPPORT_OPENGL) == PFD_SUPPORT_OPENGL)?"PFD_SUPPORT_OPENGL ":"", 
				((pfd.dwFlags & PFD_SUPPORT_GDI) == PFD_SUPPORT_GDI)?"PFD_SUPPORT_GDI ":"", 
				((pfd.dwFlags & PFD_GENERIC_FORMAT) == PFD_GENERIC_FORMAT)?"PFD_GENERIC_FORMAT ":"",
				((pfd.dwFlags & PFD_GENERIC_ACCELERATED) == PFD_GENERIC_ACCELERATED)?"PFD_GENERIC_ACCELERATED ":"",
				((pfd.dwFlags & PFD_SUPPORT_DIRECTDRAW) == PFD_SUPPORT_DIRECTDRAW)?"PFD_SUPPORT_DIRECTDRAW ":"",
				((pfd.dwFlags & PFD_DOUBLEBUFFER) == PFD_DOUBLEBUFFER)?"PFD_DOUBLEBUFFER ":"",
				((pfd.dwFlags & PFD_DRAW_TO_WINDOW) == PFD_DRAW_TO_WINDOW)?"PFD_DRAW_TO_WINDOW ":""
			));
	}
*/
	ReleaseDC(hwnd, hdc);
	DestroyWindow(hwnd);
	WIN_FlushMessageQueue();
#endif
} CATCH("acceleratedGL", )
	return accel;
}

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
