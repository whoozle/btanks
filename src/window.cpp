
/* Battle Tanks Game
 * Copyright (C) 2006-2007 Battle Tanks team
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
#include "finder.h"

IMPLEMENT_SINGLETON(Window, IWindow);

#include <SDL/SDL_opengl.h>
#ifndef SDL_OPENGLBLIT
#define SDL_OPENGLBLIT 0
// using 0 as OPENGLBLIT value. SDL 1.3 or later
#endif

IWindow::IWindow() : _fr(10.0f) {}

void IWindow::init(const int argc, char *argv[]) {
#ifdef __linux__
//	putenv("SDL_VIDEODRIVER=dga");
#endif

	_opengl = true;
	
	bool fullscreen = false;
	bool dx = false;
	bool vsync = false;
	int fsaa = 0;
	bool force_soft = false;

	int w = 800, h = 600;
	int bits = 0;
	
	for(int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "--no-gl") == 0) _opengl = false;
		else if (strcmp(argv[i], "--fs") == 0) fullscreen = true;
		else if (strcmp(argv[i], "--vsync") == 0) vsync = true;
#ifdef WIN32
		else if (strcmp(argv[i], "--dx") == 0) { dx = true; _opengl = false; }
#endif
		else if (strcmp(argv[i], "--320x200") == 0) { w = 320; h = 200; }
		else if (strcmp(argv[i], "--320x240") == 0) { w = 320; h = 240; }
		else if (strcmp(argv[i], "-0") == 0) { w = 640; h = 480; }
		else if (strcmp(argv[i], "-1") == 0) { w = 800; h = 600; }
		else if (strcmp(argv[i], "-2") == 0) { w = 1024; h = 768; }
		else if (strcmp(argv[i], "-3") == 0) { w = 1152; h = 864; }
		else if (strcmp(argv[i], "-4") == 0) { w = 1280; h = 1024; }
		else if (strcmp(argv[i], "--force-16") == 0) { bits = 16; }
		else if (strcmp(argv[i], "--fsaa") == 0) { fsaa = (fsaa)?(fsaa<< 1) : 1; }
		else if (strcmp(argv[i], "--force-soft-gl") == 0) { force_soft = true; }
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
	Uint32 subsystems = SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK;
#ifdef DEBUG
	sdlx::System::init(subsystems | SDL_INIT_NOPARACHUTE);
#else
	sdlx::System::init(subsystems);
#endif

	SDL_EnableUNICODE(1);

	if (_opengl) {
		LOG_DEBUG(("loading GL library"));
		if (SDL_GL_LoadLibrary(NULL) == -1) 
			throw_sdl(("SDL_GL_LoadLibrary"));

	}
	
	int default_flags = sdlx::Surface::Hardware | sdlx::Surface::Alpha | (_opengl? SDL_OPENGL: 0) ;
	if (_opengl) {
		default_flags &= ~SDL_OPENGL;
#ifdef USE_GLSDL
		default_flags |= SDL_GLSDL;
#endif
	}

	sdlx::Surface::setDefaultFlags(default_flags);

	LOG_DEBUG(("initializing SDL_ttf..."));
	sdlx::TTF::init();

	int flags = SDL_HWSURFACE | (bits == 0)? SDL_ANYFORMAT: 0;
	flags |= SDL_DOUBLEBUF;
	
	if (fullscreen) flags |= SDL_FULLSCREEN;

#if 0
#ifdef WIN32
	LOG_DEBUG(("loading icon..."));
	{
		sdlx::Surface icon;
		icon.loadFromResource(MAKEINTRESOURCE(1));
		SDL_WM_SetIcon(icon.getSDLSurface(), NULL);
	}
#endif
#endif

	std::string icon_file = Finder->find("tiles/icon.png", false);
	if (!icon_file.empty()) {
		TRY {
			sdlx::Surface icon;
			icon.loadImage(icon_file);
			SDL_WM_SetIcon(icon.getSDLSurface(), NULL);
		} CATCH("setting icon", {});
	}

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

#ifdef WIN32
		//SIGSEGV in SDL under linux if no GLX visual present. (debian sid, fc6)
		r = SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1);
		if (r == -1) 
			LOG_WARN(("cannot set SDL_GL_ACCELERATED_VISUAL."));
#endif

#endif
		
		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
		//SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
		
		if (fsaa > 0) {
			LOG_DEBUG(("fsaa mode: %d", fsaa));
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, fsaa);
		}
	
		//_window.setVideoMode(w, h, 0,  SDL_OPENGL | SDL_OPENGLBLIT | flags );
#ifdef USE_GLSDL
		flags |= SDL_GLSDL;
#endif

		_window.setVideoMode(w, h, bits, flags );

#if SDL_VERSION_ATLEAST(1,2,10)

		int accel = 0;
		if ((r = SDL_GL_GetAttribute( SDL_GL_ACCELERATED_VISUAL, &accel)) == 0) {
			LOG_DEBUG(("SDL_GL_ACCELERATED_VISUAL = %d", accel));

		
			if (!force_soft && accel != 1) {
				throw_ex(("Looks like you don't have a graphics card that is good enough.\n"
				"Please ensure that your graphics card supports OpenGL and the latest drivers are installed.\n" 
				"Try --force-soft-gl switch to enable sofware GL renderer."
				"Or use --no-gl to switch disable GL renderer completely."
				));
			}
		} else LOG_WARN(("SDL_GL_GetAttribute( SDL_GL_ACCELERATED_VISUAL) failed: %s (%d)", SDL_GetError(), r));
#endif

	} else {
		_window.setVideoMode(w, h, bits, flags);
	}
	if (bits != 0 && bits != _window.getBPP())
		throw_ex(("cannot setup video mode to %d bits (got: %d)", bits, _window.getBPP()));
	
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

	_running = true;
}

void IWindow::run() {

	GET_CONFIG_VALUE("engine.fps-limit", int, fps_limit, 1000);
	
	_fr = fps_limit;
	int max_delay = 1000000 / fps_limit;
	LOG_DEBUG(("fps_limit set to %d, maximum frame delay: %d", fps_limit, max_delay));

	SDL_Event event;
	while (_running) {
		_timer.reset();
		
		while (SDL_PollEvent(&event)) {
			event_signal.emit(event);
		
			switch(event.type) {
			case SDL_JOYBUTTONDOWN:
				joy_button_signal.emit(event.jbutton.which, event.jbutton.button, event.jbutton.type == SDL_JOYBUTTONDOWN);
			break;
			
			case SDL_KEYUP:			
			case SDL_KEYDOWN:
				key_signal.emit(event.key.keysym, event.type == SDL_KEYDOWN);
			break;
			
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				mouse_signal.emit(event.button.button, event.button.type == SDL_MOUSEBUTTONDOWN, event.button.x, event.button.y);
				break;
			
			case SDL_MOUSEMOTION:
				mouse_motion_signal.emit(event.motion.state, event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
				break;
			
		    case SDL_QUIT:
				_running = false;
			break;
    		}
		}
		
		const float dt = 1.0/_fr;
		
		tick_signal.emit(dt);
		
		IWindow::flip();

		int t_delta = _timer.microdelta();

		if (t_delta < max_delay) {
			//LOG_DEBUG(("tdelta: %d, delay: %d", t_delta, max_delay - t_delta));
			sdlx::Timer::microsleep(max_delay - t_delta);
		}

		t_delta = _timer.microdelta();
		_fr = (t_delta != 0)? (1000000.0 / t_delta): 1000000;
	}
	LOG_DEBUG(("exiting main loop."));
	if (_running)
		throw_sdl(("SDL_WaitEvent"));

}

void IWindow::deinit() {
	_running = false;
	LOG_DEBUG(("shutting down, freeing surface"));
	_window.free();
}

IWindow::~IWindow() {
	//_window.free();
}

void IWindow::flip() {
	_window.flip();
	if (_opengl) {
		//glFlush_ptr.call();
	}
}

void IWindow::resetTimer() {
	_timer.reset();
}
