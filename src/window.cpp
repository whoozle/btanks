
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
#include "window.h"
#include "config.h"
#include "sdlx/system.h"
#include "sdlx/ttf.h"
#include "sdlx/sdl_ex.h"
#include "version.h"

#include <SDL/SDL_opengl.h>
#ifndef SDL_OPENGLBLIT
#define SDL_OPENGLBLIT 0
// using 0 as OPENGLBLIT value. SDL 1.3 or later
#endif


void Window::init(const int argc, char *argv[]) {
#ifdef __linux__
//	putenv("SDL_VIDEODRIVER=dga");
#endif

	_opengl = true;
	
	bool fullscreen = false;
	bool dx = false;
	bool vsync = false;

	int w = 800, h = 600;
	
	for(int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "--no-gl") == 0) _opengl = false;
		else if (strcmp(argv[i], "--fs") == 0) fullscreen = true;
		else if (strcmp(argv[i], "--vsync") == 0) vsync = true;
#ifdef WIN32
		else if (strcmp(argv[i], "--dx") == 0) { dx = true; _opengl = false; }
#endif
		else if (strcmp(argv[i], "-0") == 0) { w = 640; h = 480; }
		else if (strcmp(argv[i], "-1") == 0) { w = 800; h = 600; }
		else if (strcmp(argv[i], "-2") == 0) { w = 1024; h = 768; }
		else if (strcmp(argv[i], "-3") == 0) { w = 1152; h = 864; }
		else if (strcmp(argv[i], "-4") == 0) { w = 1280; h = 1024; }
		else if (strcmp(argv[i], "--help") == 0) { 
			printf(
					"\t--no-gl\t\t\tdisable GL renderer\n"
					"\t--dx\t\t\tenable directX(tm) renderer (win32 only)\n"
					"\t-2 -3\t\t\tenlarge video mode to 1024x768 or 1280x1024\n"
				  );
			exit(0);
		}
	}
	
	LOG_DEBUG(("gl: %s, vsync: %s, dx: %s", _opengl?"yes":"no", vsync?"yes":"no", dx?"yes":"no"));
#ifdef WIN32
	_putenv("SDL_VIDEO_RENDERER=gdi");

	if (dx) 
#if SDL_MAJOR_VERSION >= 1 && SDL_MINOR_VERSION >= 3
		_putenv("SDL_VIDEO_RENDERER=d3d");
#else
		_putenv("SDL_VIDEODRIVER=directx");
#endif

#endif

//opengl renderer
#if SDL_MAJOR_VERSION >= 1 && SDL_MINOR_VERSION >= 3
	if (_opengl)
		_putenv("SDL_VIDEO_RENDERER=opengl");
#endif

	LOG_DEBUG(("initializing SDL..."));
#ifdef DEBUG
	sdlx::System::init(SDL_INIT_EVERYTHING | SDL_INIT_NOPARACHUTE);
#else
	sdlx::System::init(SDL_INIT_EVERYTHING);
#endif

	if (_opengl) {
		LOG_DEBUG(("loading GL library"));
		if (SDL_GL_LoadLibrary(NULL) == -1) 
			throw_sdl(("SDL_GL_LoadLibrary"));

	}
	
	int default_flags = sdlx::Surface::Hardware | sdlx::Surface::Alpha | (_opengl? SDL_OPENGL: 0) ;
#ifdef USE_GLSDL
	if (_opengl) {
		default_flags &= ~SDL_OPENGL;
		default_flags |= SDL_GLSDL;
	}
#endif

	sdlx::Surface::setDefaultFlags(default_flags);

	LOG_DEBUG(("initializing SDL_ttf..."));
	sdlx::TTF::init();

	int flags = SDL_HWSURFACE | SDL_ANYFORMAT;
	//if (doublebuf)
	flags |= SDL_DOUBLEBUF;
	
	if (fullscreen) flags |= SDL_FULLSCREEN;

	LOG_DEBUG(("setting caption..."));		
	SDL_WM_SetCaption(("Battle tanks - " + getVersion()).c_str(), "btanks");


	if (_opengl) {
#if SDL_VERSION_ATLEAST(1,2,10)
		LOG_DEBUG(("setting GL swap control to %d...", vsync?1:0));
		int r = SDL_GL_SetAttribute( SDL_GL_SWAP_CONTROL, vsync?1:0);
		if (r == -1) 
			LOG_WARN(("cannot set SDL_GL_SWAP_CONTROL."));
#ifdef WIN32
		if (!vsync) {
			typedef void (APIENTRY * WGLSWAPINTERVALEXT) (int);
			WGLSWAPINTERVALEXT wglSwapIntervalEXT = (WGLSWAPINTERVALEXT) 
			wglGetProcAddress("wglSwapIntervalEXT");
			if (wglSwapIntervalEXT) {
				LOG_DEBUG(("disabling vsync with SwapIntervalEXT(0)..."));
			    wglSwapIntervalEXT(0); // disable vertical synchronisation
			}
		}
#endif

		LOG_DEBUG(("setting GL accelerated visual..."));

		r = SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
		if (r == -1) 
			LOG_WARN(("cannot set SDL_GL_ACCELERATED_VISUAL."));
#endif
		
		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
		//SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
	
	
		//_window.setVideoMode(w, h, 0,  SDL_OPENGL | SDL_OPENGLBLIT | flags );
#ifdef USE_GLSDL
		flags |= SDL_GLSDL;
#endif
		_window.setVideoMode(w, h, 0, flags );
	} else {
		_window.setVideoMode(w, h, 0, flags);
	}
	
	LOG_DEBUG(("created main surface. (%dx%dx%d, %s)", w, h, _window.getBPP(), ((_window.getFlags() & SDL_HWSURFACE) == SDL_HWSURFACE)?"hardware":"software"));

	sdlx::System::probeVideoMode();	
#if 0
	{
		SDL_Rect **modes;
		int i;

		/* Get available fullscreen/hardware modes */
		modes = SDL_ListModes(NULL, SDL_FULLSCREEN|SDL_HWSURFACE);

		/* Check is there are any modes available */
		if(modes == (SDL_Rect **)0) 
			throw_ex(("No video modes available"));
    
	    /* Check if our resolution is restricted */
    	if(modes == (SDL_Rect **)-1){
			LOG_DEBUG(("all resolutions available."));
		} else {
			/* Print valid modes */
			LOG_DEBUG(("available modes:"));
			for(i=0;modes[i];++i)
				LOG_DEBUG(("\t%dx%d", modes[i]->w, modes[i]->h));
		}
	}
#endif
}

void Window::deinit() {
	LOG_DEBUG(("shutting down, freeing surface"));
	_window.free();
}

Window::~Window() {
	//_window.free();
}

void Window::flip() {
	_window.flip();
	if (_opengl) {
		//glFlush_ptr.call();
	}
}
