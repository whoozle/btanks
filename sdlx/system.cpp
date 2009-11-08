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


#include "sdlx/system.h"
#include "sdlx/sdlx.h"
#include "sdlx/sdl_ex.h"
#include <stdlib.h>

#include "mrt/logger.h"

using namespace sdlx;

#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static void WIN_FlushMessageQueue()
{
	MSG  msg;
	while ( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) ) {
		if ( msg.message == WM_QUIT ) break;
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}
}
#elif defined __APPLE__
	//nothing here
#else
#	include <X11/X.h>
#	include <X11/Xutil.h>
#	include <GL/glx.h>
#	include <GL/gl.h>
#	include <GL/glu.h>

template <typename FuncPtr> union union_ptr {
	FuncPtr func;
	void *ptr;
	union_ptr() { memset(this, 0, sizeof(*this));}
};

#endif

const bool System::accelerated_gl(const bool windowed) {
	bool accel = true;
	LOG_DEBUG(("checking for accelerating GL..."));
TRY {
#if 0
//#ifdef _WINDOWS
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
#if !defined(_WINDOWS) && !defined(__APPLE__)
	int errorBase, eventBase; 
	
	if (SDL_GL_LoadLibrary(NULL) != 0) {
		LOG_WARN(("SDL_GL_LoadLibrary failed: %s", SDL_GetError()));
		return false;
	}
	
	union_ptr<Bool (APIENTRY *)( Display *dpy, int *, int *)> glx_query_ext;
	if ((glx_query_ext.ptr = SDL_GL_GetProcAddress("glXQueryExtension")) == NULL)
		throw_ex(("no glXQueryExtension in GL library"));
	union_ptr<XVisualInfo* (APIENTRY *)(Display *, int, int *)> glx_choose_visual;
	if ((glx_choose_visual.ptr = SDL_GL_GetProcAddress("glXChooseVisual")) == NULL)
		throw_ex(("no glXChooseVisual in GL library"));
	union_ptr<GLXContext (APIENTRY *)(Display *, XVisualInfo *, GLXContext, Bool)> glx_create_context;
	if ((glx_create_context.ptr = SDL_GL_GetProcAddress("glXCreateContext")) == NULL)
		throw_ex(("no glXCreateContext in GL library"));
	union_ptr<Bool (APIENTRY *)(Display *, GLXContext)> glx_is_direct;
	if ((glx_is_direct.ptr = SDL_GL_GetProcAddress("glXIsDirect")) == NULL)
		throw_ex(("no glXIsDirect in GL library"));
	union_ptr<void (APIENTRY *)(Display *, GLXContext)> glx_destroy_context;
	if ((glx_destroy_context.ptr = SDL_GL_GetProcAddress("glXDestroyContext")) == NULL)
		throw_ex(("no glXDestroyContext in GL library"));

	accel = false;
	Display *display = XOpenDisplay(NULL);

    static int doubleBufferVisual[]  =
    {
        GLX_RGBA,           // Needs to support OpenGL
        GLX_DEPTH_SIZE, 16, // Needs to support a 16 bit depth buffer
        GLX_DOUBLEBUFFER,   // Needs to support double-buffering
        None                // end of list
    };

	if (display == NULL)
		goto end;
	
	if (!glx_query_ext.func(display, &errorBase, &eventBase))
		goto end;

{
    XVisualInfo *visual_info = glx_choose_visual.func(display, DefaultScreen(display), doubleBufferVisual);
	if (visual_info == NULL)
		goto end;
	
	GLXContext gl_context = glx_create_context.func(display, visual_info, NULL, GL_TRUE);
	if (gl_context == NULL) 
		goto end;
	
	accel = glx_is_direct.func(display, gl_context);
	LOG_DEBUG(("direct rendering: %s", accel? "yes":"no"));

	glx_destroy_context.func(display, gl_context);
}
	
end: 
	XCloseDisplay(display);
	
#endif
} CATCH("accelerated_gl", {})
	return accel;
}

void System::init(int system) {
	LOG_DEBUG(("calling SDL_init('%08x')", (unsigned)system));
	if (SDL_Init(system) == -1) 
		throw_sdl(("SDL_Init"));
}

void System::probe_video_mode() {
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
